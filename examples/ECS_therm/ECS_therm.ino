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


long lastMsg = 0;

void processEcsStateMsg(char* topic, byte* payload, unsigned int length)
{
	
  Serial.print("Checking if state topic");
  if(String(topic) == "ECS/state"){
	  Serial.println("Received ECS/State change message");
	  if ((char)payload[0] == '1') {
		Serial.println("ECS ON received");
		
	  }
	  else{
		  Serial.println("ECS OFF received");
	  }
	  
  }
  else{
	  Serial.println("not for me...");
  }

	
}


void setup() {
  Serial.begin(115200);
  myMqtt = new MqttConnection(SENSOR_ID, WLAN_SSID, WLAN_PASS, MQTT_SERVER, MQTT_PORT);
  myMqtt->registerCustomProcessing(&processEcsStateMsg);
  myMqtt->addSubscription("state");
  
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

