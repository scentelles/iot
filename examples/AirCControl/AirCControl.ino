
#include <ModbusRTU.h>  
//#include <pthread.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <string.h>

#include <ArduinoOTA.h>

#ifdef TELNET_DEBUG 
  #include <RemoteDebug.h> //https://github.com/JoaoLopesF/RemoteDebug v2.1.2
#endif

//#include <ESPmDNS.h>

#include "MqttConnection.h"
#include "gree.h"
#include "servo.h"

MqttConnection * myMqtt;


/************************* WiFi Access Point *********************************/

#define WLAN_SSID       "SFR_34A8"
#define WLAN_PASS       "ab4ingrograstanstorc"


#define SENSOR_ID "AC"

#ifdef LOLIN
  #define LED_PIN 22 //for lolin32 lite
#endif
#ifdef WEMOSMINI
  #define LED_PIN 2
#endif
#ifdef ESP32_DEVKIT
  #define LED_PIN 2
#endif

#define AERO_IDLE             "1"
#define AERO_CONFIG_ONGOING   "2"
#define AERO_CONFIGURED       "3"


bool servoRunning[NB_SERVO];
int positionArray[NB_SERVO];
int positionTargetArray[NB_SERVO];

#define LOOP_PERIOD 10


unsigned long time_now = 0;

enum ModbusCmdType : uint8_t
{
  MODBUS_CMD_SET_MODE,
  MODBUS_CMD_SET_POWER,
  MODBUS_CMD_SET_FAN_SPEED,
  MODBUS_CMD_SET_TEMPERATURE,
  MODBUS_CMD_SET_SILENT,
  MODBUS_CMD_SET_NRJ_SAVING,
  MODBUS_CMD_SET_TEMP_LOWER_LIMIT_NRJ,
  MODBUS_CMD_SET_TEMP_UPPER_LIMIT_NRJ,
  MODBUS_CMD_READ_CORE,
  MODBUS_CMD_READ_SECONDARY,
  MODBUS_CMD_READ_COILS
};

struct ModbusCommand
{
  ModbusCmdType type;
  int value;
};

struct MqttPublish
{
  char leafTopic[64];
  char payload[32];
};

QueueHandle_t modbusQueue = NULL;
QueueHandle_t mqttQueue = NULL;
TaskHandle_t modbusTaskHandle = NULL;
TaskHandle_t mqttTaskHandle = NULL;

volatile uint32_t mqttDropCount = 0;

const int MQTT_PUBLISH_BURST_MAX = 5;
const unsigned long SERVO_PUBLISH_INTERVAL_MS = 300;

unsigned long lastServoPublishMs[NB_SERVO] = {0};

void taskModbus(void * param);
void taskMqttWifi(void * param);

bool enqueueModbusCommand(ModbusCmdType type, int value)
{
  if (modbusQueue == NULL)
  {
    return false;
  }
  ModbusCommand cmd = {type, value};
  return xQueueSend(modbusQueue, &cmd, 0) == pdTRUE;
}

bool enqueueMqttPublish(const char * leafTopic, const char * payload)
{
  if (mqttQueue == NULL)
  {
    return false;
  }
  MqttPublish msg;
  strncpy(msg.leafTopic, leafTopic, sizeof(msg.leafTopic) - 1);
  msg.leafTopic[sizeof(msg.leafTopic) - 1] = '\0';
  strncpy(msg.payload, payload, sizeof(msg.payload) - 1);
  msg.payload[sizeof(msg.payload) - 1] = '\0';
  if (xQueueSend(mqttQueue, &msg, 0) == pdTRUE) {
    return true;
  }
  mqttDropCount++;
  return false;
}





/************************* MQTT *********************************/

#define MQTT_SERVER  "192.168.1.27"
#define MQTT_PORT 1883


int loopCounter = 0;
bool bootComplete = false;
bool endOfConfigRequestedFromHost = false;






//============================

bool isAnyServoRunning()
{
   for (int i = 0; i < NB_SERVO; i++)
   {
       if(servoRunning[i] == true)
       {
         return true; 
       }  
   }
   return false;
}


void processACMsg(char* topic, byte* payload, unsigned int length)
{
 
 //debugPrintln("Received custom command");
 String strPayload = "";
 for (int i = 0; i < length; i++)
 {
     strPayload += (char)payload[i];
 }
 int intPayload = atoi(strPayload.c_str());

 
  if(String(topic) == "AC/ESP/HOST_INIT_REQUEST"){
        debugPrintln("Received HOST_INIT_REQUEST !!!!!!!!!!!!!!!!!!");

        initPositions(); 
  } 
  else if(String(topic) == "AC/ESP/PING")
  {
      debugPrintln("Ping received.replying");
      if (Serial && Serial.availableForWrite() > 32) {
        Serial.println("--------------pong");
      }
      char pongPayload[32];
      if (strPayload.length() > 0) {
        snprintf(pongPayload, sizeof(pongPayload), "%s|%lu", strPayload.c_str(), millis());
      } else {
        snprintf(pongPayload, sizeof(pongPayload), "%lu", millis());
      }
      enqueueMqttPublish("ESP/PONG", pongPayload);
     
  }
  else if(String(topic) == "AC/GREE/mode/set")
  {
      debugPrintln(String("COMMAND : MODE SET : " + strPayload).c_str());
      if(strPayload == "COOL")
      {
          debugPrintln("\tMODBUS SET MODE : COOL");
          enqueueModbusCommand(MODBUS_CMD_SET_MODE, GREE_MODE_COOL);
      }
      else if(strPayload == "HEAT")
      {
          debugPrintln("\tMODBUS SET MODE : HEAT");
          enqueueModbusCommand(MODBUS_CMD_SET_MODE, GREE_MODE_HEAT);
      }
      else if(strPayload == "FAN")
      {
          debugPrintln("\tMODBUS SET MODE : FAN");
          enqueueModbusCommand(MODBUS_CMD_SET_MODE, GREE_MODE_FAN);
      }
      else if(strPayload == "DRY")
      {
          debugPrintln("\tMODBUS SET MODE : DRY");
          enqueueModbusCommand(MODBUS_CMD_SET_MODE, GREE_MODE_DRY);
      }
      else
      {
        
        debugPrint("========== ERROR : UNKOWN MODE");
      }
     
  }
  else if(String(topic) == "AC/GREE/power/set")
  {
      debugPrintln(String("COMMAND : POWER SET : " + strPayload).c_str());
      if(intPayload == 1)
      {
        enqueueModbusCommand(MODBUS_CMD_SET_POWER, 1);
      }
      else if(intPayload == 0)
      {
        enqueueModbusCommand(MODBUS_CMD_SET_POWER, 0);
      }
      else
      {
        debugPrint("ERROR : unknown power set command\n");
      }
     
  }
  else if(String(topic) == "AC/GREE/fanspeed/set")
  {
      debugPrintln(String("COMMAND : FANSPEED SET : " + strPayload).c_str());
      enqueueModbusCommand(MODBUS_CMD_SET_FAN_SPEED, intPayload);
  }
  else if(String(topic) == "AC/GREE/temperature/set")
  {
      debugPrintln(String("COMMAND : TEMPERATURE SET : " + strPayload).c_str());
      enqueueModbusCommand(MODBUS_CMD_SET_TEMPERATURE, intPayload);
  }
  else if(String(topic) == "AC/GREE/silent/set")
  {
      debugPrintln(String("COMMAND : SILENT MODE SET : " + strPayload).c_str());
      enqueueModbusCommand(MODBUS_CMD_SET_SILENT, intPayload);
  }


  else if(String(topic) == "AC/GREE/templowerlimitnrj/set")
  {
      debugPrintln(String("COMMAND : NRJSAVING TEMPERATURE LOWER LIMIT NRJ SET : " + strPayload).c_str());
      enqueueModbusCommand(MODBUS_CMD_SET_TEMP_LOWER_LIMIT_NRJ, intPayload);
  }
  else if(String(topic) == "AC/GREE/tempupperlimitnrj/set")
  {
      debugPrintln(String("COMMAND : NRJSAVING TEMPERATURE UPPER LIMIT NRJ SET : " + strPayload).c_str());
      enqueueModbusCommand(MODBUS_CMD_SET_TEMP_UPPER_LIMIT_NRJ, intPayload);
  }  
  else if(String(topic) == "AC/GREE/nrjsaving/set")
  {
      debugPrintln(String("COMMAND : NRJSAVING SET : " + strPayload).c_str());
      enqueueModbusCommand(MODBUS_CMD_SET_NRJ_SAVING, intPayload);
  }
  else if(String(topic) == "AC/GREE/corestatus/get")
  {
      myMqtt->unsubscribe("GREE/corestatus/get"); //unsubscribe to avoid beeing flood

      if(isAnyServoRunning() == false)
      {
        debugPrintln("COMMAND : CORE STATUS GET REQUEST");
        enqueueModbusCommand(MODBUS_CMD_READ_CORE, 0);
      }
      myMqtt->subscribe("GREE/corestatus/get"); 
  }
  else if(String(topic) == "AC/GREE/secondarystatus/get")
  {
      myMqtt->unsubscribe("GREE/secondarystatus/get"); //unsubscribe to avoid beeing flood
      if(isAnyServoRunning() == false)
      {
        debugPrintln("COMMAND : SECONDARY STATUS GET REQUEST");
        enqueueModbusCommand(MODBUS_CMD_READ_SECONDARY, 0);
      }
      myMqtt->unsubscribe("GREE/secondarystatus/get"); 
  }
  else if(String(topic) == "AC/GREE/coils/get")
  {
      debugPrintln("COMMAND : COILS GET REQUEST");
      enqueueModbusCommand(MODBUS_CMD_READ_COILS, 0);

  }  
   else if(String(topic) == "AC/ESP/SERVO/CHAMBRE1/ANGLE"){ debugPrint("Received Angle setting : "); debugPrintln(strPayload.c_str()); positionTargetArray[SERVO_CHAMBRE1] = intPayload; servoRunning[SERVO_CHAMBRE1] = true;}
   else if(String(topic) == "AC/ESP/SERVO/CHAMBRE2/ANGLE"){ debugPrint("Received Angle setting : "); debugPrintln(strPayload.c_str()); positionTargetArray[SERVO_CHAMBRE2] = intPayload; servoRunning[SERVO_CHAMBRE2] = true;}
   else if(String(topic) == "AC/ESP/SERVO/CHAMBRE3/ANGLE"){ debugPrint("Received Angle setting : "); debugPrintln(strPayload.c_str()); positionTargetArray[SERVO_CHAMBRE3] = intPayload; servoRunning[SERVO_CHAMBRE3] = true;}
   else if(String(topic) == "AC/ESP/SERVO/DREAMROOM/ANGLE"){debugPrint("Received Angle setting : "); debugPrintln(strPayload.c_str()); positionTargetArray[SERVO_DREAMROOM] = intPayload;servoRunning[SERVO_DREAMROOM] = true;}      
   else if(String(topic) == "AC/ESP/SERVO/ETAGE/ANGLE"){    debugPrint("Received Angle setting : "); debugPrintln(strPayload.c_str()); positionTargetArray[SERVO_ETAGE] = intPayload;    servoRunning[SERVO_ETAGE] = true;} 
   else if(String(topic) == "AC/ESP/SERVO/MASTER2/ANGLE"){  debugPrint("Received Angle setting : "); debugPrintln(strPayload.c_str()); positionTargetArray[SERVO_MASTER2] = intPayload;  servoRunning[SERVO_MASTER2] = true;} 		 
	 else if(String(topic) == "AC/ESP/SERVO/SALON/ANGLE"){    debugPrint("Received Angle setting : "); debugPrintln(strPayload.c_str()); positionTargetArray[SERVO_SALON] = intPayload;    servoRunning[SERVO_SALON] = true;} 	 
		 
		 else {
		    debugPrint("Unknown topic : ");
			  debugPrintln(String(topic).c_str());
		 }

}

void initBoot()
{
     enqueueMqttPublish("ESP/INIT_DONE", "1");
  
}
void initPositions()
{
   debugPrintln("Starting initialization of all servos");
   
    for(int servoId = 0; servoId < NB_SERVO; servoId++)
  {
    positionArray[servoId] = 0;
    positionTargetArray [servoId] = 0;
    positionLoopStartTime[servoId] = 0;
    servoRunning[servoId] = false;
    String leafTopic = String("ESP/SERVO/") + ID_TO_ROOM[servoId] + "/REAL_ANGLE";
    enqueueMqttPublish(leafTopic.c_str(), "0");
  }
  // delay(5000);
  debugPrintln("Starting init position");
  unsigned long   initStartTime = millis();
  while(millis() - initStartTime < NB_SECONDS_FOR_90_DEGREES * 1000)
  {
    for(int servoId = 0; servoId < NB_SERVO; servoId++)
    {
      if(servoId != SERVO_SAFETY_CHANNEL)
      {
        digitalWrite(ID_TO_SERVOR[servoId], LOW);
        digitalWrite(ID_TO_SERVOL[servoId], HIGH) ; 
      }
    }
    digitalWrite(ID_TO_SERVOR[SERVO_SAFETY_CHANNEL], HIGH);
    digitalWrite(ID_TO_SERVOL[SERVO_SAFETY_CHANNEL], LOW) ; 
    blinkLED(50);
    myMqtt->loop(); //run mqttloop while initializing servos, as we are out of main loop
    debugPrint(".");
    #ifdef TELNET_DEBUG 
    Debug.handle();
    #endif
  }
  for(int servoId = 0; servoId < NB_SERVO; servoId++)
  {
    //if(servoId != SERVO_ETAGE)
    //{
      digitalWrite(ID_TO_SERVOR[servoId], LOW);
      digitalWrite(ID_TO_SERVOL[servoId], LOW) ; 
    //}
  }
        
  debugPrintln("Servo init Done");
  enqueueMqttPublish("ESP/INIT_SERVO_DONE", "1");
}


void blinkLED(int value)
{
  digitalWrite(LED_PIN, LOW);
  delay(value);
  digitalWrite(LED_PIN, HIGH); 
  delay(value);

}

void initOTA()
{
  
  ArduinoOTA.setHostname("ESP_32_AC_GREE");

  ArduinoOTA
    .onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH)
        type = "sketch";
      else // U_SPIFFS
        type = "filesystem";

      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
      debugPrintln(String("Start updating " + type).c_str());
    })
    .onEnd([]() {
      debugPrintln("\nEnd");
    })
    .onProgress([](unsigned int progress, unsigned int total) {
      debugPrintln(String("Progress: " +  (progress / (total / 100))).c_str());
    })
    .onError([](ota_error_t error) {
      debugPrintln(String("Error: " + error).c_str());
      if (error == OTA_AUTH_ERROR) debugPrintln("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) debugPrintln("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) debugPrintln("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) debugPrintln("Receive Failed");
      else if (error == OTA_END_ERROR) debugPrintln("End Failed");
    });

  ArduinoOTA.begin();

}

void setup() {
  delay(1000); //delay to wait power supply to stabilize
  #ifndef TELNET_DEBUG  
    Serial.begin(115200);
  #endif

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW); 
  delay(200);
  digitalWrite(LED_PIN, HIGH); 
  delay(200);
  digitalWrite(LED_PIN, LOW); 
  delay(200);
  digitalWrite(LED_PIN, HIGH); 
    
  Serial2.begin(9600, SERIAL_8N1, UART_RX, UART_TX);
  mb.begin(&Serial2, UART_RXTX);

  mb.master();

  initServoPins();

 
  myMqtt = new MqttConnection(SENSOR_ID, WLAN_SSID, WLAN_PASS, MQTT_SERVER, MQTT_PORT);
  myMqtt->registerCustomProcessing(&processACMsg);
  myMqtt->addSubscription("ESP/SERVO/CHAMBRE1/ANGLE");
  myMqtt->addSubscription("ESP/SERVO/CHAMBRE2/ANGLE");
  myMqtt->addSubscription("ESP/SERVO/CHAMBRE3/ANGLE");
  myMqtt->addSubscription("ESP/SERVO/DREAMROOM/ANGLE");
  myMqtt->addSubscription("ESP/SERVO/ETAGE/ANGLE");
  myMqtt->addSubscription("ESP/SERVO/SALON/ANGLE");
  myMqtt->addSubscription("ESP/SERVO/MASTER2/ANGLE");
  myMqtt->addSubscription("ESP/HOST_INIT_REQUEST"); 
  myMqtt->addSubscription("ESP/PING"); 
  myMqtt->addSubscription("GREE/mode/set");   
  myMqtt->addSubscription("GREE/power/set");   
  myMqtt->addSubscription("GREE/fanspeed/set");  
  myMqtt->addSubscription("GREE/temperature/set");  
  myMqtt->addSubscription("GREE/corestatus/get"); 
  myMqtt->addSubscription("GREE/secondarystatus/get"); 
  myMqtt->addSubscription("GREE/coils/get"); 
  myMqtt->addSubscription("GREE/silent/set"); 
  myMqtt->addSubscription("GREE/templowerlimitnrj/set"); 
  myMqtt->addSubscription("GREE/tempupperlimitnrj/set"); 
  myMqtt->addSubscription("GREE/nrjsaving/set"); 



  initCoils();
  initHregs();

  initOTA();

  // init remote debug
  #ifdef TELNET_DEBUG 
  Debug.begin("ESP32");  
  #endif 
  
  WiFi.setSleep(false);

  modbusQueue = xQueueCreate(10, sizeof(ModbusCommand));
  mqttQueue = xQueueCreate(50, sizeof(MqttPublish));

  if (modbusQueue != NULL && mqttQueue != NULL)
  {
    xTaskCreatePinnedToCore(taskModbus, "ModbusTask", 4096, NULL, 1, &modbusTaskHandle, 1);
    xTaskCreatePinnedToCore(taskMqttWifi, "MqttTask", 8192, NULL, 2, &mqttTaskHandle, 1);
  }
  
}


void turn(int servoId, bool turnRight)
{
    if(turnRight)
    {
      digitalWrite(ID_TO_SERVOR[servoId], HIGH);
      digitalWrite(ID_TO_SERVOL[servoId], LOW);
    }
    else
    {
      digitalWrite(ID_TO_SERVOR[servoId], LOW);
      digitalWrite(ID_TO_SERVOL[servoId], HIGH) ;     
    }

  
    if(millis() - positionLoopStartTime[servoId] < TIMING_PER_DEGREE)
    {
      //positionLoopStartTime[servoId] = ;
    }
    else
    {
      positionLoopStartTime[servoId] = millis();

      if(turnRight)
      {
        positionArray[servoId] += 1;
        logServo("TURN RIGHT : ");
      }
      else
      {
        positionArray[servoId] -= 1;
        logServo("TURN LEFT : ");        
      }
     logServo(String(servoId));
     logServo("  : ");
     logServoCR(String(positionArray[servoId]));
      unsigned long now = millis();
      if (now - lastServoPublishMs[servoId] >= SERVO_PUBLISH_INTERVAL_MS) {
        char payload[12];
        snprintf(payload, sizeof(payload), "%d", positionArray[servoId]);
        String leafTopic = String("ESP/SERVO/") + ID_TO_ROOM[servoId] + "/REAL_ANGLE";
        enqueueMqttPublish(leafTopic.c_str(), payload);
        lastServoPublishMs[servoId] = now;
      }
    }
}

void turnOff(int servoId)
{
    if(servoRunning[servoId] == true)
    {
      positionLoopStartTime[servoId] = 0;
      servoRunning[servoId]=false;
      digitalWrite(ID_TO_SERVOR[servoId], LOW);
      digitalWrite(ID_TO_SERVOL[servoId], LOW);
      logServo("TURN OFF : ");
      logServoCR(String(servoId));
     }
}




const char * mqttStateToString(int state)
{
  switch (state)
  {
    case -4: return "MQTT_CONNECTION_TIMEOUT";
    case -3: return "MQTT_CONNECTION_LOST";
    case -2: return "MQTT_CONNECT_FAILED";
    case -1: return "MQTT_DISCONNECTED";
    case 0: return "MQTT_CONNECTED";
    case 1: return "MQTT_CONNECT_BAD_PROTOCOL";
    case 2: return "MQTT_CONNECT_BAD_CLIENT_ID";
    case 3: return "MQTT_CONNECT_UNAVAILABLE";
    case 4: return "MQTT_CONNECT_BAD_CREDENTIALS";
    case 5: return "MQTT_CONNECT_UNAUTHORIZED";
    default: return "MQTT_STATE_UNKNOWN";
  }
}

int loopCount = 0;
bool ledHigh = false;
unsigned long lastWifiReconnectAttempt = 0;

void blinkLedShort(int nbBlink)
{
  for(int i = 0; i < nbBlink; i++)
    if(ledHigh)
    {
       ledHigh = false;
       digitalWrite(LED_PIN, LOW);
    }
    else
    {
       ledHigh = true;
       digitalWrite(LED_PIN, HIGH);
    }
    delay(100);
}

void taskModbus(void * param)
{
  ModbusCommand cmd;
  for(;;)
  {
    if (xQueueReceive(modbusQueue, &cmd, portMAX_DELAY) == pdTRUE)
    {
      switch(cmd.type)
      {
        case MODBUS_CMD_SET_MODE:
          greeSetMode(cmd.value);
          break;
        case MODBUS_CMD_SET_POWER:
          greeSetPower(cmd.value != 0);
          break;
        case MODBUS_CMD_SET_FAN_SPEED:
          greeSetFanSpeed(cmd.value);
          break;
        case MODBUS_CMD_SET_TEMPERATURE:
          greeSetTemperature(cmd.value);
          break;
        case MODBUS_CMD_SET_SILENT:
          greeSetSilent(cmd.value != 0);
          break;
        case MODBUS_CMD_SET_NRJ_SAVING:
          greeSetNRJSaving(cmd.value != 0);
          break;
        case MODBUS_CMD_SET_TEMP_LOWER_LIMIT_NRJ:
          greeSetTemperatureLowerLimitNrj(cmd.value);
          break;
        case MODBUS_CMD_SET_TEMP_UPPER_LIMIT_NRJ:
          greeSetTemperatureUpperLimitNrj(cmd.value);
          break;
        case MODBUS_CMD_READ_CORE:
          readModbusCoreValues();
          sendModbusCoreValues();
          break;
        case MODBUS_CMD_READ_SECONDARY:
          readModbusSecondaryValues();
          sendModbusSecondaryValues();
          break;
        case MODBUS_CMD_READ_COILS:
          readAllCoils();
          break;
        default:
          break;
      }
    }
  }
}

void taskMqttWifi(void * param)
{
  MqttPublish msg;
  unsigned long lastTaskTick = 0;
  for(;;)
  {
    if(WiFi.status() == WL_CONNECTED) {
      unsigned long now = millis();
      if (lastTaskTick != 0 && now - lastTaskTick > 2000) {
        Serial.print("SERIAL : MQTT TASK GAP=");
        Serial.print(now - lastTaskTick);
        Serial.print("ms QUEUE=");
        if (mqttQueue != NULL) {
          Serial.print(uxQueueMessagesWaiting(mqttQueue));
        } else {
          Serial.print("NA");
        }
        Serial.print(" DROPS=");
        Serial.print(mqttDropCount);
        Serial.print(" WIFI=");
        Serial.println((int)WiFi.status());
      }
      lastTaskTick = now;
      ArduinoOTA.handle();
      #ifdef TELNET_DEBUG 
      Debug.handle();
      #endif
      loopCount++;

      if(loopCount > 2000)
      {
        loopCount = 0;
        debugPrintln("AC Alive, looping\n");
        Serial.println("Alive, looping\n");
        Serial.print("SERIAL : MQTT DROPS=");
        Serial.println(mqttDropCount);
        unsigned long delta = millis() - time_now;
        time_now = millis(); 
        debugPrintln(String(delta).c_str());
        if(ledHigh)
        {
           ledHigh = false;
           digitalWrite(LED_PIN, LOW);
        }
        else
        {
           ledHigh = true;
           digitalWrite(LED_PIN, HIGH);
        }
      }
      


      if (!myMqtt->connected()) {
        debugPrintln("MQTT RECONNECT!!!!!!!!!!!!!!!!!!!!!");
        Serial.println("SERIAL : MQTT RECONNECT!!!!!!!!!!!!!!!!!!!!!");
        int mqttState = myMqtt->state();
        wl_status_t wifiStatus = WiFi.status();
        Serial.print("SERIAL : MQTT STATE=");
        Serial.print(mqttState);
        Serial.print(" (");
        Serial.print(mqttStateToString(mqttState));
        Serial.print(")");
        Serial.print(" WIFI=");
        Serial.print((int)wifiStatus);
        if (wifiStatus == WL_CONNECTED) {
          Serial.print(" RSSI=");
          Serial.print(WiFi.RSSI());
        }
        Serial.println();
        if (myMqtt->reconnect()) {
          blinkLedShort(5);
        }
      }
      
      myMqtt->loop();
      
      
        if(bootComplete == false)
        {
          initBoot();
          bootComplete = true;
        }

      if (myMqtt->connected())
      {
        int publishCount = 0;
        while (publishCount < MQTT_PUBLISH_BURST_MAX && xQueueReceive(mqttQueue, &msg, 0) == pdTRUE)
        {
          myMqtt->publishValue(msg.leafTopic, msg.payload);
          myMqtt->loop();
          publishCount++;
        }
      }

      for(int servoId = 0; servoId < NB_SERVO; servoId++)
      {
        if(positionArray[servoId] < positionTargetArray[servoId])
        {
          turn(servoId, true);
        }
        else 
        {
          if(positionArray[servoId] > positionTargetArray[servoId])
          {
            turn(servoId, false);
          }
          else
          {
            turnOff(servoId);
          }
        }



      }
    }
    else
    {
      //debugPrintln("Wifi reconnect ongoing");
      Serial.println("SERIAL : Wifi reconnect ongoing");
      unsigned long now = millis();
      if (now - lastWifiReconnectAttempt >= 5000) {
        lastWifiReconnectAttempt = now;
        WiFi.reconnect();
      }
      vTaskDelay(pdMS_TO_TICKS(100));
      continue;
    }
    vTaskDelay(pdMS_TO_TICKS(1));
  }
}


void loop() {
  vTaskDelay(pdMS_TO_TICKS(1000));
}
