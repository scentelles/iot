
#include <ModbusRTU.h>  
//#include <pthread.h>

#include <ArduinoOTA.h>

#include <RemoteDebug.h>

//#include <ESPmDNS.h>

#include "MqttConnection.h"
#include "gree.h"
#include "servo.h"

MqttConnection * myMqtt;

RemoteDebug Debug;



/************************* WiFi Access Point *********************************/

#define WLAN_SSID       "SFR_34A8"
#define WLAN_PASS       "ab4ingrograstanstorc"


#define SENSOR_ID "AC"

#define LED_PIN 22

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



#define TELNET_DEBUG
void debugPrintln(const char * msg)
{
#ifdef TELNET_DEBUG    
    Debug.println(F(msg));
#else
    debugPrintln(msg);
#endif
}
void debugPrint(const char * msg)
{
#ifdef TELNET_DEBUG    
    Debug.print(F(msg));
#else
    debugPrint(msg);
#endif
}



//============================

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
      debugPrintln("COMMAND : MODE SET");
      if(strPayload == "COOL")
      {
          greeSetMode(GREE_MODE_COOL);
      }
      else
      {
        debugPrint("========== ERROR : UNKOWN MODE");
      }
     
  }
  else if(String(topic) == "AC/GREE/power/set")
  {
      debugPrintln("COMMAND : POWER SET");
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
      debugPrintln("COMMAND : FANSPEED SET");
      greeSetFanSpeed(intPayload);
  }
  else if(String(topic) == "AC/GREE/temperature/set")
  {
      debugPrintln("COMMAND : TEMPERATURE SET");
      greeSetTemperature(intPayload * 10) ;
  }
  else if(String(topic) == "AC/GREE/corestatus/get")
  {
      debugPrintln("COMMAND : CORE STATUS GET REQUEST");
      readModbusCoreValues();
      sendModbusCoreValues(myMqtt);

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


void initPositions()
{
    for(int servoId = 0; servoId < NB_SERVO; servoId++)
  {
    positionArray[servoId] = 0;
    positionTargetArray [servoId] = 0;
    positionLoopStartTime[servoId] = 0;
    servoRunning[servoId] = false;
    myMqtt->publishValue(String("ESP/SERVO/" + ID_TO_ROOM[servoId] + "/REAL_ANGLE").c_str(), "0");
  }
  // delay(5000);
   myMqtt->publishValue("ESP/INIT_DONE", "1");
}

void blinkLED()
{
  digitalWrite(LED_PIN, LOW);
  delay(200);
  digitalWrite(LED_PIN, HIGH); 
  delay(200);

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
  Serial.begin(115200);


 
  pinMode(LED_PIN, OUTPUT);


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
  myMqtt->addSubscription("ESP/SERVO/RUN_ALL");  
  myMqtt->addSubscription("ESP/HOST_INIT_REQUEST"); 
  myMqtt->addSubscription("ESP/PING"); 
  myMqtt->addSubscription("GREE/mode/set");   
  myMqtt->addSubscription("GREE/power/set");   
  myMqtt->addSubscription("GREE/fanspeed/set");  
  myMqtt->addSubscription("GREE/temperature/set");  
  myMqtt->addSubscription("GREE/corestatus/get"); 



  initCoils();
  initHregs();

  initOTA();

  // init remote debug
  Debug.begin("ESP32");  
  
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

bool allServoConfigured()
{
    for(int servoId = 0; servoId < NB_SERVO; servoId++)
  {
    if(servoRunning[servoId])
    {
      return false;
    }
  }
  return true;
}

int loopCount = 0;
bool ledHigh = false;
void loop() {

  ArduinoOTA.handle();
  Debug.handle();

  loopCount++;
  if(loopCount > 500)
  {
    loopCount = 0;

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
   // readAllCoils();
    //readModbusCoreValues();
  }
  
    time_now = millis();
  // put your main code here, to run repeatedly:

  if (!myMqtt->connected()) {
    myMqtt->reconnect();
  }
  myMqtt->loop();

    if(bootComplete == false)
    {
      initPositions();
      bootComplete = true;
    }

  if(endOfConfigRequestedFromHost)
  {
     if(allServoConfigured())
     {
       endOfConfigRequestedFromHost = false;
       myMqtt->publishValue("ESP/AERAULIC_STATE", AERO_CONFIGURED);
       
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

   /*
    while(millis() < time_now + LOOP_PERIOD){
        //wait approx. [period] ms
    }*/


}
