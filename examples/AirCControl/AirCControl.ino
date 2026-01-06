
#include <ModbusRTU.h>  
//#include <pthread.h>

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
      myMqtt->publishValue("ESP/PONG", "1");
     
  }
  else if(String(topic) == "AC/GREE/mode/set")
  {
      debugPrintln(String("COMMAND : MODE SET : " + strPayload).c_str());
      if(strPayload == "COOL")
      {
          debugPrintln("\tMODBUS SET MODE : COOL");
          greeSetMode(GREE_MODE_COOL);
      }
      else if(strPayload == "HEAT")
      {
          debugPrintln("\tMODBUS SET MODE : HEAT");
          greeSetMode(GREE_MODE_HEAT);
      }
      else if(strPayload == "FAN")
      {
          debugPrintln("\tMODBUS SET MODE : FAN");
          greeSetMode(GREE_MODE_FAN);
      }
      else if(strPayload == "DRY")
      {
          debugPrintln("\tMODBUS SET MODE : DRY");
          greeSetMode(GREE_MODE_DRY);
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
        greeSetPower(true);
      }
      else if(intPayload == 0)
      {
        greeSetPower(false);        
      }
      else
      {
        debugPrint("ERROR : unknown power set command\n");
      }
     
  }
  else if(String(topic) == "AC/GREE/fanspeed/set")
  {
      debugPrintln(String("COMMAND : FANSPEED SET : " + strPayload).c_str());
      greeSetFanSpeed(intPayload);
  }
  else if(String(topic) == "AC/GREE/temperature/set")
  {
      debugPrintln(String("COMMAND : TEMPERATURE SET : " + strPayload).c_str());
      greeSetTemperature(intPayload) ;
  }
  else if(String(topic) == "AC/GREE/silent/set")
  {
      debugPrintln(String("COMMAND : SILENT MODE SET : " + strPayload).c_str());
      greeSetSilent((bool)intPayload) ;
  }


  else if(String(topic) == "AC/GREE/templowerlimitnrj/set")
  {
      debugPrintln(String("COMMAND : NRJSAVING TEMPERATURE LOWER LIMIT NRJ SET : " + strPayload).c_str());
      greeSetTemperatureLowerLimitNrj(intPayload) ;
  }
  else if(String(topic) == "AC/GREE/tempupperlimitnrj/set")
  {
      debugPrintln(String("COMMAND : NRJSAVING TEMPERATURE UPPER LIMIT NRJ SET : " + strPayload).c_str());
      greeSetTemperatureUpperLimitNrj(intPayload) ;
  }  
  else if(String(topic) == "AC/GREE/nrjsaving/set")
  {
      debugPrintln(String("COMMAND : NRJSAVING SET : " + strPayload).c_str());
      greeSetNRJSaving((bool)intPayload) ;
  }
  else if(String(topic) == "AC/GREE/corestatus/get")
  {
      myMqtt->unsubscribe("GREE/corestatus/get"); //unsubscribe to avoid beeing flood

      if(isAnyServoRunning() == false)
      {
        debugPrintln("COMMAND : CORE STATUS GET REQUEST");
        readModbusCoreValues();
        delay(10);//Debug check if maybe to early.
        sendModbusCoreValues(myMqtt);
      }
      myMqtt->subscribe("GREE/corestatus/get"); 
  }
  else if(String(topic) == "AC/GREE/secondarystatus/get")
  {
      myMqtt->unsubscribe("GREE/secondarystatus/get"); //unsubscribe to avoid beeing flood
      if(isAnyServoRunning() == false)
      {
        debugPrintln("COMMAND : SECONDARY STATUS GET REQUEST");
        readModbusSecondaryValues();
        sendModbusSecondaryValues(myMqtt);
      }
      myMqtt->unsubscribe("GREE/secondarystatus/get"); 
  }
  else if(String(topic) == "AC/GREE/coils/get")
  {
      debugPrintln("COMMAND : COILS GET REQUEST");
      readAllCoils();

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
     myMqtt->publishValue("ESP/INIT_DONE", "1");
  
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
    myMqtt->publishValue(String("ESP/SERVO/" + ID_TO_ROOM[servoId] + "/REAL_ANGLE").c_str(), "0");
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
  myMqtt->publishValue("ESP/INIT_SERVO_DONE", "1");
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
      myMqtt->publishValue(String("ESP/SERVO/" + ID_TO_ROOM[servoId] + "/REAL_ANGLE").c_str(), String(positionArray[servoId]).c_str());
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




int loopCount = 0;
bool ledHigh = false;
int nbTry = 0;
int wifi_timeout_counter = 0;

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



void loop() {


  if(WiFi.status() == WL_CONNECTED) {
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
    myMqtt->reconnect();
    blinkLedShort(5);
    nbTry++;
    if(nbTry > 20)
    {
      ESP.restart();
    }
  }
  nbTry = 0;
  
  myMqtt->loop();
  
  
    if(bootComplete == false)
    {
      initBoot();
      bootComplete = true;
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
    delay(100);  
    wifi_timeout_counter++;
    if(wifi_timeout_counter > 200) // 20secs
    {
        ESP.restart();
        wifi_timeout_counter = 0;
    }
    
  }
   /*
    while(millis() < time_now + LOOP_PERIOD){
        //wait approx. [period] ms
    }*/


}
