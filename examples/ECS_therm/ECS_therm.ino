#include "MqttConnection.h"
MqttConnection * myMqtt;

#include <OneWire.h>
#include <DallasTemperature.h>

//Constants
#define SENSOR_ID "ECS"
#define PROBE_TEMPO 60000
#define ONE_WIRE_BUS D4  // DS18B20 pin
#define RELAY_PIN D2
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature DS18B20(&oneWire);


/************************* WiFi Access Point *********************************/

#define WLAN_SSID       "SFR_34A8"
#define WLAN_PASS       "ab4ingrograstanstorc"


/************************* MQTT *********************************/


#define MQTT_SERVER  "192.168.1.27"
#define MQTT_PORT 1883

#define ECS_FORCE_DISABLED 	'0'
#define ECS_FORCE_OFF 		'1'
#define ECS_FORCE_ON  		'2'
#define ECS_STATE_ON		'2'
#define ECS_STATE_OFF		'1'


long lastMsg = 0;
char ecsForce = ECS_FORCE_DISABLED;
char ecsState = ECS_STATE_OFF;
char msg[2];

void processEcsStateMsg(char* topic, byte* payload, unsigned int length)
{
	
  Serial.print("Checking if state topic");
  if(String(topic) == "ECS/state"){
	  Serial.println("Received ECS/State change message");
	  if(ecsForce == ECS_FORCE_DISABLED)
	  {
		 Serial.println("changing ECS relay state");
	     if ((char)payload[0] == ECS_STATE_ON) {
		    Serial.println("ECS ON received");
			ecsState = ECS_STATE_ON;
	     }
	     else{
		    Serial.println("ECS OFF received");
			ecsState = ECS_STATE_OFF;
	     }
     }
	  else
	 {
			Serial.println("FORCED is set. No action on relay. Rolling back state on MQTT server");
			//Rewrite current state only if delta with current state. Else we go into infinite loop...
			  if(ecsState != (char)payload[0])
			  {       
		              msg[0] = ecsState;
					  msg[1] = 0;
					  myMqtt->publish("ECS/state", msg);
			  }
	 }
	  
  }
  else if(String(topic) == "ECS/force"){
	  Serial.println("Received ECS/State change message");
	  
	  if ((char)payload[0] == ECS_FORCE_OFF) {
		Serial.println("Force OFF received");
	    msg[0] = ECS_STATE_OFF;
		msg[1] = 0;
		myMqtt->publish("ECS/state", msg);
		//TODO : ugly
	    delay(1000);
		ecsForce = ECS_FORCE_OFF;

	  }
	  else if ((char)payload[0] == ECS_FORCE_ON) {
		Serial.println("Force ON received");
		msg[0] = ECS_STATE_ON;
		msg[1] = 0;
		myMqtt->publish("ECS/state", msg);
		//TODO : ugly
		delay(1000);
		ecsForce = ECS_FORCE_ON;
	  }
	  else
	  {
		  Serial.println("Force disabled");
		  ecsForce = ECS_FORCE_DISABLED;
	  }
  }
  else {
	  Serial.println("not for me...");
  }  

	
}


void setup() {
  Serial.begin(115200);
  myMqtt = new MqttConnection(SENSOR_ID, WLAN_SSID, WLAN_PASS, MQTT_SERVER, MQTT_PORT);
  myMqtt->registerCustomProcessing(&processEcsStateMsg);
  myMqtt->addSubscription("state");
  myMqtt->addSubscription("force");  
}

void loop() {
  float temp;
  if (!myMqtt->connected()) {
    myMqtt->reconnect();
  }
  myMqtt->loop();


  long now = millis(); 

  if (now - lastMsg > PROBE_TEMPO) {
      lastMsg = now;
      
      //Read data and store it to variables hum and temp
      DS18B20.requestTemperatures(); 
      temp = DS18B20.getTempCByIndex(0);
      Serial.print("Temperature: ");
      Serial.println(temp);
      
      if (temp == 85.0 || temp == (-127.0)) {
          Serial.println("Failed to read from sensor!");
      }
      else
      {
          //send to MQTT server
          myMqtt->publishValue("temp", temp, 1);    
      }
  }


  

}

