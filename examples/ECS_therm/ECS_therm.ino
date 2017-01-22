#include "MqttConnection.h"
MqttConnection * myMqtt;

#include <OneWire.h>
#include <DallasTemperature.h>

//Constants
const char ECS_STATE_OFF = '1';
const char ECS_STATE_ON  = '2';

#define SENSOR_ID "ECS2"
#define PROBE_TEMPO 60000  //1 measure per minute
#define ONE_WIRE_BUS D4  // DS18B20 pin
#define RELAY_PIN D1
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature DS18B20(&oneWire);


/************************* WiFi Access Point *********************************/

#define WLAN_SSID       "SFR_34A8"
#define WLAN_PASS       "ab4ingrograstanstorc"


/************************* MQTT *********************************/


#define MQTT_SERVER  "192.168.1.27"
#define MQTT_PORT 1883

#define DELAY_BETWEEN_TOGGLE_ON 9000 //00  //15min
#define DELAY_BETWEEN_TOGGLE_OFF 900 //000  //15min

long lastMsg = 0;
char ecsState = ECS_STATE_OFF;
char msg[2];
unsigned long lastSwitchOn = 0;
unsigned long lastSwitchOff = 0;

bool checkToggleTooFast(bool switchOn){
  Serial.print("now :");
  Serial.println(millis());
  Serial.print("last on:");
  Serial.println(lastSwitchOn);  
  Serial.print("last off:");
  Serial.println(lastSwitchOff);   
  if(switchOn){
      if(millis() - lastSwitchOff < DELAY_BETWEEN_TOGGLE_ON)
         return true;
      else
         return false;
   }
   else{
      if(millis() - lastSwitchOn < DELAY_BETWEEN_TOGGLE_OFF)
         return true;
      else
         return false;
  
   
   }
      
}

void processEcsStateMsg(char* topic, byte* payload, unsigned int length)
{

  Serial.print("Checking if state topic");
 // if(String(topic) == "ECS/state"){
	  Serial.println("Received ECS/State change message");
		 Serial.println("changing ECS relay state");
	     if ((char)payload[0] == ECS_STATE_ON) {
		    Serial.println("ECS ON received");
						
			if(!checkToggleTooFast(true)){
			    ecsState = ECS_STATE_ON;
			    digitalWrite(RELAY_PIN, HIGH);
				lastSwitchOn = millis();
			}
		    else
			    Serial.println("Skipping, switching too fast");
	     }
	     else if ((char)payload[0] == ECS_STATE_OFF){
		    Serial.println("ECS OFF received");

			if(!checkToggleTooFast(false)){
			   ecsState = ECS_STATE_OFF;
			   digitalWrite(RELAY_PIN, LOW);
			   lastSwitchOff = millis();
			}
	     }
		 else {
		    Serial.print("Unknown payload : ");
			Serial.println((char)payload[0]);
		 }

	  
  //}
  //else {
  //	  Serial.println("not for me...");
  //}  

	
}


void setup() {
  Serial.begin(115200);
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);

  
  myMqtt = new MqttConnection(SENSOR_ID, WLAN_SSID, WLAN_PASS, MQTT_SERVER, MQTT_PORT);
  myMqtt->registerCustomProcessing(&processEcsStateMsg);
  myMqtt->addSubscription("state");
  myMqtt->addSubscription("force");  
  
  
}

void loop() {
  float temp1, temp2;
  if (!myMqtt->connected()) {
	Serial.println("still not connected, retrying");
    myMqtt->reconnect();
  }
  myMqtt->loop();


  long now = millis(); 

  if (now - lastMsg > PROBE_TEMPO) {
      lastMsg = now;
      
      //Read data and store it to variables hum and temp
      DS18B20.requestTemperatures(); 
      temp1 = DS18B20.getTempCByIndex(0);
      Serial.print("Temperature: ");
      Serial.println(temp1);
      temp2 = DS18B20.getTempCByIndex(1);
      Serial.print("Temperature: ");
      Serial.println(temp2);      
      if (temp1 == 85.0 || temp1 == (-127.0) || temp2 == 85.0 || temp2 == (-127.0)) {
          Serial.println("Failed to read from sensor!");
      }
      else
      {
          //send to MQTT server
          myMqtt->publishValue("temp1", temp1, 1);    
		  myMqtt->publishValue("temp2", temp2, 1);    
      }
  }


  

}
