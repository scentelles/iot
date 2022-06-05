#include "MqttConnection.h"
MqttConnection * myMqtt;


/************************* WiFi Access Point *********************************/

#define WLAN_SSID       "SFR_34A8"
#define WLAN_PASS       "ab4ingrograstanstorc"

#define SENSOR_ID "ALARM1"

#define ALARM_LED_RELAY_PIN        D5
#define ALARM_BUZ_RELAY_PIN        D1 



/************************* MQTT *********************************/

#define MQTT_SERVER  "192.168.1.27"
#define MQTT_PORT 1883

//Constants
const char ALARM_LED_ON  =  '2';
const char ALARM_LED_OFF =  '1';
const char ALARM_BUZ_ON  =  '4';
const char ALARM_BUZ_OFF =  '3';
void processAlarmStateMsg(char* topic, byte* payload, unsigned int length)
{

  Serial.print("Checking if state topic");
  if(String(topic) == "ALARM1/cmd"){
	  Serial.println("Received Alarm change message");
       if ((char)payload[0] == ALARM_LED_ON){
		    Serial.println("ALARM LED ON received");

			  switchAlarmLedOn();
			
	     }
       else if ((char)payload[0] == ALARM_LED_OFF){
        Serial.println("ALARM LED OFF received");

        switchAlarmLedOff();
      
       }
       else if ((char)payload[0] == ALARM_BUZ_ON){
        Serial.println("ALARM BUZ ON received");

        switchAlarmBuzOn();
      
       }
       else if ((char)payload[0] == ALARM_BUZ_OFF){
        Serial.println("ALARM BUZ OFF received");

        switchAlarmBuzOff();
      
       }
		 else {
		    Serial.print("Unknown payload : ");
			Serial.println((char)payload[0]);
		 }

  }
}

void setup() {
  Serial.begin(115200);
  pinMode(ALARM_LED_RELAY_PIN, OUTPUT);
  digitalWrite(ALARM_LED_RELAY_PIN, LOW);
  pinMode(ALARM_BUZ_RELAY_PIN, OUTPUT);
  digitalWrite(ALARM_BUZ_RELAY_PIN, LOW);
  
  delay(10);
  
  myMqtt = new MqttConnection(SENSOR_ID, WLAN_SSID, WLAN_PASS, MQTT_SERVER, MQTT_PORT);
  myMqtt->registerCustomProcessing(&processAlarmStateMsg);
  myMqtt->addSubscription("cmd");

}


void switchAlarmLedOn(){
    
  Serial.println("Switching Alarm LED ON");
  digitalWrite(ALARM_LED_RELAY_PIN, HIGH);

}
void switchAlarmLedOff(){
    
  Serial.println("Switching Alarm LED OFF");
  digitalWrite(ALARM_LED_RELAY_PIN, LOW);

}
void switchAlarmBuzOn(){
    
  Serial.println("Switching Alarm BUZ ON");
  digitalWrite(ALARM_BUZ_RELAY_PIN, HIGH);

}
void switchAlarmBuzOff(){
    
  Serial.println("Switching Alarm BUZ OFF");
  digitalWrite(ALARM_BUZ_RELAY_PIN, LOW);

}
void loop() {
  // put your main code here, to run repeatedly:

  if (!myMqtt->connected()) {
    myMqtt->reconnect();
  }
  myMqtt->loop();

 
  
  delay(10);


}
