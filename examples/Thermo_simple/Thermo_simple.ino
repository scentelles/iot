#include "MqttConnection.h"
MqttConnection * myMqtt;

#include <OneWire.h>
#include <DallasTemperature.h>

#define SENSOR_ID "AQUA"
#define PROBE_TEMPO 60000  //1 measure per minute
#define ONE_WIRE_BUS D5  // DS18B20 pin

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature DS18B20(&oneWire);


/************************* WiFi Access Point *********************************/

#define WLAN_SSID       "SFR_34A8"
#define WLAN_PASS       "ab4ingrograstanstorc"


/************************* MQTT *********************************/


#define MQTT_SERVER  "192.168.1.27"
#define MQTT_PORT 1883


long lastMsg = 0;

char msg[2];



void setup() {
  Serial.begin(115200);
  
  myMqtt = new MqttConnection(SENSOR_ID, WLAN_SSID, WLAN_PASS, MQTT_SERVER, MQTT_PORT);
 
  
}

void loop() {
  float temp;
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
      temp = DS18B20.getTempCByIndex(0);
      Serial.print("Temperature: ");
      Serial.println(temp);
    
      if (temp == 85.0 || temp == (-127.0) ) {
          Serial.println("Failed to read from sensor!");
      }
      else
      {
          //send to MQTT server
          myMqtt->publishValue("temp", temp, 1);    
  
      }
  }


  

}
