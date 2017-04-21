//Libraries

#include "MqttConnection.h"
MqttConnection * myMqtt;

//Constants
#define SENSOR_ID "FLOWER1_WATER"
#define SLEEP_TIME_S 300
#define SENSOR_PIN A0     // what pin we're connected to

//Variables


float moisture_level;  //Stores moisture value


/************************* WiFi Access Point *********************************/

#define WLAN_SSID       "SFR_34A8"
#define WLAN_PASS       "ab4ingrograstanstorc"

/************************* MQTT *********************************/

#define MQTT_SERVER  "192.168.1.27"
#define MQTT_PORT 1883

/************ Global State (you don't need to change this!) ******************/

long lastMsg = 0;

/*************************** Sketch Code ************************************/


void setup()
{
  Serial.begin(115200);
  myMqtt = new MqttConnection(SENSOR_ID, WLAN_SSID, WLAN_PASS, MQTT_SERVER, MQTT_PORT);

  pinMode(SENSOR_PIN, INPUT);
  
}


void loop()
{
  if (!myMqtt->connected()) {
    myMqtt->reconnect();
  }
  myMqtt->loop();


 
  int analogSensor = analogRead(SENSOR_PIN);
  Serial.print("Moisture : ");
  Serial.println(analogSensor);
  myMqtt->publishValue("moisture", analogSensor, 1);
  
  delay(SLEEP_TIME_S *1000);
}

   
