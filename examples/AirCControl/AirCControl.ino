
#include <ModbusRTU.h>  
#include <pthread.h>

#include "MqttConnection.h"
#include "gree.h"
#include "servo.h"

MqttConnection * myMqtt;




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
int positionLoopCounter[NB_SERVO];


#define LOOP_PERIOD 10
unsigned long time_now = 0;

/************************* MQTT *********************************/

#define MQTT_SERVER  "192.168.1.27"
#define MQTT_PORT 1883


int loopCounter = 0;
bool bootComplete = false;
bool endOfConfigRequestedFromHost = false;

//Modbus methods//==========
bool cbWrite(Modbus::ResultCode event, uint16_t transactionId, void* data) {
  Serial.printf_P("Request result: 0x%02X, Mem: %d\n", event, ESP.getFreeHeap());
  return true;
}
//============================

void processACMsg(char* topic, byte* payload, unsigned int length)
{
 
 //Serial.println("Received custom command");
 String strPayload = "";
 for (int i = 0; i < length; i++)
 {
     strPayload += (char)payload[i];
 }
 int intPayload = atoi(strPayload.c_str());

 
  if(String(topic) == "AC/ESP/SERVO/RUN_ALL"){
        int servoId = (char)payload[0];
		    Serial.println("Received SERVO RUN ALL ");
        endOfConfigRequestedFromHost = true;

  }
  else if(String(topic) == "AC/ESP/HOST_INIT_REQUEST"){
        Serial.println("Received HOST_INIT_REQUEST !!!!!!!!!!!!!!!!!!");

        initPositions(); 
  } 
  else if(String(topic) == "AC/ESP/PING")
  {
      Serial.println("Ping received.replying");
      blinkLED();
      myMqtt->publishValue("ESP/PONG", "1");
     
  }
  else if(String(topic) == "AC/GREE/mode/set")
  {
      Serial.println("COMMAND : MODE SET");
      if(strPayload == "COOL")
      {
          greeSetMode(GREE_MODE_COOL);
      }
      else
      {
        Serial.print("========== ERROR : UNKOWN MODE");
      }
     
  }
  else if(String(topic) == "AC/GREE/power/set")
  {
      Serial.println("COMMAND : POWER SET");
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
        Serial.print("ERROR : unknown power set command\n");
      }
     
  }
  else if(String(topic) == "AC/GREE/fanspeed/set")
  {
      Serial.println("COMMAND : FANSPEED SET");
      greeSetFanSpeed(intPayload);
  }
  else if(String(topic) == "AC/GREE/temperature/set")
  {
      Serial.println("COMMAND : TEMPERATURE SET");
      greeSetTemperature(intPayload * 10) ;
  }
  else if(String(topic) == "AC/GREE/corestatus/get")
  {
      Serial.println("COMMAND : CORE STATUS GET REQUEST");
      readModbusCoreValues();
      sendModbusCoreValues(myMqtt);

  }

  
   else if(String(topic) == "AC/ESP/SERVO/CHAMBRE1/ANGLE"){ Serial.print("Received Angle setting : "); Serial.println(intPayload); positionTargetArray[SERVO_CHAMBRE1] = intPayload; servoRunning[SERVO_CHAMBRE1] = true;}
   else if(String(topic) == "AC/ESP/SERVO/CHAMBRE2/ANGLE"){ Serial.print("Received Angle setting : "); Serial.println(intPayload); positionTargetArray[SERVO_CHAMBRE2] = intPayload; servoRunning[SERVO_CHAMBRE2] = true;}
   else if(String(topic) == "AC/ESP/SERVO/CHAMBRE3/ANGLE"){ Serial.print("Received Angle setting : "); Serial.println(intPayload); positionTargetArray[SERVO_CHAMBRE3] = intPayload; servoRunning[SERVO_CHAMBRE3] = true;}
   else if(String(topic) == "AC/ESP/SERVO/DREAMROOM/ANGLE"){Serial.print("Received Angle setting : "); Serial.println(intPayload); positionTargetArray[SERVO_DREAMROOM] = intPayload;servoRunning[SERVO_DREAMROOM] = true;}      
   else if(String(topic) == "AC/ESP/SERVO/ETAGE/ANGLE"){    Serial.print("Received Angle setting : "); Serial.println(intPayload); positionTargetArray[SERVO_ETAGE] = intPayload;    servoRunning[SERVO_ETAGE] = true;} 
   else if(String(topic) == "AC/ESP/SERVO/MASTER2/ANGLE"){  Serial.print("Received Angle setting : "); Serial.println(intPayload); positionTargetArray[SERVO_MASTER2] = intPayload;  servoRunning[SERVO_MASTER2] = true;} 		 
	 else if(String(topic) == "AC/ESP/SERVO/SALON/ANGLE"){    Serial.print("Received Angle setting : "); Serial.println(intPayload); positionTargetArray[SERVO_SALON] = intPayload;    servoRunning[SERVO_SALON] = true;} 	 
		 
		 else {
		    Serial.print("Unknown topic : ");
			  Serial.println(String(topic));
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
      positionLoopCounter[servoId] = 0;
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
void loop() {

  loopCount++;
  if(loopCount > 100)
  {
    loopCount = 0;
   // readAllCoils();
    readModbusCoreValues();
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

  //delay(LOOP_DELAY);


   
    //Serial.println("Hello");
   
    while(millis() < time_now + LOOP_PERIOD){
        //wait approx. [period] ms
    }


}
