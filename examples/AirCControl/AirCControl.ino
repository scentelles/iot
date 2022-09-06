
  
#include <pthread.h>

#include "MqttConnection.h"


MqttConnection * myMqtt;




/************************* WiFi Access Point *********************************/

#define WLAN_SSID       "SFR_34A8"
#define WLAN_PASS       "ab4ingrograstanstorc"

#define SENSOR_ID "AC"


#define SERVO1_TURNL_PIN        D1
#define SERVO1_TURNR_PIN        D2 
#define SERVO2_TURNL_PIN        D1
#define SERVO2_TURNR_PIN        D2 
#define SERVO3_TURNL_PIN        D1
#define SERVO3_TURNR_PIN        D2 
#define SERVO4_TURNL_PIN        D1
#define SERVO4_TURNR_PIN        D2 

#define SERVO_MASTER1_TURNL_PIN        D1
#define SERVO_MASTER1_TURNR_PIN        D2 
#define SERVO_MASTER1_TURNL_PIN        D1
#define SERVO_MASTER1_TURNR_PIN        D2 
#define SERVO_MASTER1_TURNL_PIN        D1
#define SERVO_MASTER1_TURNR_PIN        D2 

#define SERVO_CHAMBRE1  0
#define SERVO_CHAMBRE2  1
#define SERVO_CHAMBRE3  2
#define SERVO_DREAMROOM 3
#define SERVO_ETAGE     4
#define SERVO_SALON     5
#define SERVO_MASTER2   6


#define AERO_IDLE             "1"
#define AERO_CONFIG_ONGOING   "2"
#define AERO_CONFIGURED       "3"

#define NB_SERVO  7

String ID_TO_ROOM[NB_SERVO];

bool servoRunning[NB_SERVO];
int positionArray[NB_SERVO];
int positionTargetArray[NB_SERVO];
int positionLoopCounter[NB_SERVO];


#define ONE_DEGREE_COUNTER 10

#define LOOP_PERIOD 10
unsigned long time_now = 0;

/************************* MQTT *********************************/

#define MQTT_SERVER  "192.168.1.27"
#define MQTT_PORT 1883


int loopCounter = 0;
bool bootComplete = false;
bool endOfConfigRequestedFromHost = false;

void processACMsg(char* topic, byte* payload, unsigned int length)
{
 
 Serial.println("Received custom command");
 String strPayload = "";
 for (int i = 0; i < length; i++)
 {
     strPayload += (char)payload[i];
 }

 
  if(String(topic) == "AC/ESP/SERVO/RUN_ALL"){
        int servoId = (char)payload[0];
		    Serial.println("Received SERVO RUN ALL ");
        endOfConfigRequestedFromHost = true;
        
        
  }
  else if(String(topic) == "AC/ESP/SERVO/RESET"){
        Serial.println("Received SERVO RESET !!!!!!!!!!!!!!!!!!");

        initPositions(); //is it really needed to wait for end of all closures?

        delay(4000);
        myMqtt->publishValue("ESP/INIT_DONE", "1");
  } 
  else if(String(topic) == "AC/ESP/PING")
  {
      Serial.println("Ping received.replying");
      myMqtt->publishValue("ESP/PONG", "1");
  }
   else if(String(topic) == "AC/ESP/SERVO/CHAMBRE1/ANGLE"){ Serial.print("Received Angle setting : "); Serial.println(atoi(strPayload.c_str())); positionTargetArray[SERVO_CHAMBRE1] = atoi(strPayload.c_str()); servoRunning[SERVO_CHAMBRE1] = true;}
   else if(String(topic) == "AC/ESP/SERVO/CHAMBRE2/ANGLE"){ Serial.print("Received Angle setting : "); Serial.println(atoi(strPayload.c_str())); positionTargetArray[SERVO_CHAMBRE2] = atoi(strPayload.c_str()); servoRunning[SERVO_CHAMBRE2] = true;}
   else if(String(topic) == "AC/ESP/SERVO/CHAMBRE3/ANGLE"){ Serial.print("Received Angle setting : "); Serial.println(atoi(strPayload.c_str())); positionTargetArray[SERVO_CHAMBRE3] = atoi(strPayload.c_str()); servoRunning[SERVO_CHAMBRE3] = true;}
   else if(String(topic) == "AC/ESP/SERVO/DREAMROOM/ANGLE"){Serial.print("Received Angle setting : "); Serial.println(atoi(strPayload.c_str())); positionTargetArray[SERVO_DREAMROOM] = atoi(strPayload.c_str());servoRunning[SERVO_DREAMROOM] = true;}      
   else if(String(topic) == "AC/ESP/SERVO/ETAGE/ANGLE"){    Serial.print("Received Angle setting : "); Serial.println(atoi(strPayload.c_str())); positionTargetArray[SERVO_ETAGE] = atoi(strPayload.c_str());    servoRunning[SERVO_ETAGE] = true;} 
   else if(String(topic) == "AC/ESP/SERVO/MASTER2/ANGLE"){  Serial.print("Received Angle setting : "); Serial.println(atoi(strPayload.c_str())); positionTargetArray[SERVO_MASTER2] = atoi(strPayload.c_str());  servoRunning[SERVO_MASTER2] = true;} 		 
	 else if(String(topic) == "AC/ESP/SERVO/SALON/ANGLE"){    Serial.print("Received Angle setting : "); Serial.println(atoi(strPayload.c_str())); positionTargetArray[SERVO_SALON] = atoi(strPayload.c_str());    servoRunning[SERVO_SALON] = true;} 	 
		 
		 else {
		    Serial.print("Unknown payload : ");
			  Serial.println((char)payload[0]);
		 }

}


void initPositions()
{
    for(int servoId = 0; servoId < NB_SERVO; servoId++)
  {
    positionArray[servoId] = 0;
    positionTargetArray [servoId] = 0;
    positionLoopCounter[servoId] = 0;
    servoRunning[servoId] = false;
    myMqtt->publishValue(String("ESP/SERVO/" + ID_TO_ROOM[servoId] + "/REAL_ANGLE").c_str(), "0");
  }

}


void setup() {
  Serial.begin(115200);
  pinMode(SERVO1_TURNL_PIN, OUTPUT);
  digitalWrite(SERVO1_TURNL_PIN, LOW);
  pinMode(SERVO2_TURNL_PIN, OUTPUT);
  digitalWrite(SERVO2_TURNL_PIN, LOW);
  
  delay(10);
  
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
  myMqtt->addSubscription("ESP/SERVO/RESET"); 
  myMqtt->addSubscription("ESP/PING"); 
  
  
  ID_TO_ROOM[SERVO_CHAMBRE1]  = "CHAMBRE1";
  ID_TO_ROOM[SERVO_CHAMBRE2]  = "CHAMBRE2";
  ID_TO_ROOM[SERVO_CHAMBRE3]  = "CHAMBRE3";
  ID_TO_ROOM[SERVO_DREAMROOM] = "DREAMROOM";
  ID_TO_ROOM[SERVO_ETAGE]     = "ETAGE";
  ID_TO_ROOM[SERVO_SALON]     = "SALON";
  ID_TO_ROOM[SERVO_MASTER2]   = "MASTER2";

  
}


void switchAlarmLedOn(){
    
  Serial.println("Switching Alarm LED ON");
 // digitalWrite(ALARM_LED_RELAY_PIN, HIGH);

}
void switchAlarmLedOff(){
    
  Serial.println("Switching Alarm LED OFF");
 // digitalWrite(ALARM_LED_RELAY_PIN, LOW);

}

void turn(int servoId, bool turnRight)
{
    if(positionLoopCounter[servoId] < ONE_DEGREE_COUNTER)
    {
      positionLoopCounter[servoId]++;
    }
    else
    {
      positionLoopCounter[servoId] = 0;

      if(turnRight)
      {
        positionArray[servoId] += 1;
        Serial.print("TURN RIGHT : ");
      }
      else
      {
        positionArray[servoId] -= 1;
        Serial.print("TURN LEFT : ");        
      }
      Serial.print(servoId);
      Serial.print("  : ");
      Serial.println(positionArray[servoId]);
      myMqtt->publishValue(String("ESP/SERVO/" + ID_TO_ROOM[servoId] + "/REAL_ANGLE").c_str(), String(positionArray[servoId]).c_str());
    }
}

void turnOff(int servoId)
{
    positionLoopCounter[servoId] = 0;
    servoRunning[servoId]=false;
  //Serial.print("TURN OFF : ");
    //Serial.println(servoId);
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

void loop() {
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

  //delay(LOOP_DELAY);


   
    //Serial.println("Hello");
   
    while(millis() < time_now + LOOP_PERIOD){
        //wait approx. [period] ms
    }


}
