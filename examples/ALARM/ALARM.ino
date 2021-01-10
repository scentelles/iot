#include "MqttConnection.h"
MqttConnection * myMqtt;


/************************* WiFi Access Point *********************************/

#define WLAN_SSID       "SFR_34A8"
#define WLAN_PASS       "ab4ingrograstanstorc"

#define SENSOR_ID "ALARM1"

#define ALARM_RELAY_PIN        D6
 



/************************* MQTT *********************************/

#define MQTT_SERVER  "192.168.1.27"
#define MQTT_PORT 1883

//Constants
const char TV_STATE_OFF = 1;
const char TV_STATE_ON  = 2;
const char ALARM_ON  =  '2';
const char ALARM_OFF =  '1';

void processAlarmStateMsg(char* topic, byte* payload, unsigned int length)
{

  Serial.print("Checking if state topic");
  if(String(topic) == "ALARM1/cmd"){
	  Serial.println("Received Alarm change message");
       if ((char)payload[0] == ALARM_ON){
		    Serial.println("ALARM ON received");

			  switchAlarmOn();
			
	     }
       else if ((char)payload[0] == ALARM_OFF){
        Serial.println("ALARM OFF received");

        switchAlarmOff();
      
       }
		 else {
		    Serial.print("Unknown payload : ");
			Serial.println((char)payload[0]);
		 }

  }
}

void setup() {
  Serial.begin(115200);
  pinMode(ALARM_RELAY_PIN, OUTPUT);
  digitalWrite(ALARM_RELAY_PIN, HIGH);

  
  delay(10);
  
  myMqtt = new MqttConnection(SENSOR_ID, WLAN_SSID, WLAN_PASS, MQTT_SERVER, MQTT_PORT);
  myMqtt->registerCustomProcessing(&processAlarmStateMsg);
  myMqtt->addSubscription("cmd");

}


void switchAlarmOn(){
    
  Serial.println("Switching Alarm ON");
  digitalWrite(ALARM_RELAY_PIN, LOW);

}
void switchAlarmOff(){
    
  Serial.println("Switching Alarm OFF");
  digitalWrite(ALARM_RELAY_PIN, HIGH);

}
void loop() {
  // put your main code here, to run repeatedly:

  if (!myMqtt->connected()) {
    myMqtt->reconnect();
  }
  myMqtt->loop();

 
  
  delay(10);


}