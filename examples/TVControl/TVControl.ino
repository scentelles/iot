#include "MqttConnection.h"
MqttConnection * myMqtt;


/************************* WiFi Access Point *********************************/

#define WLAN_SSID       "SFR_34A8"
#define WLAN_PASS       "ab4ingrograstanstorc"

#define SENSOR_ID "TV"

#define BOX_STATUS_PIN      D2    
#define AMPLIFIER_RELAY_PIN D5
#define TV_RELAY_PIN        D6
#define DELAY_BETWEEN_TOGGLE 10000
#define DELAY_BETWEEN_BOX_CHECK 10
 
#define BOX_OFF 1
#define BOX_ON  2
#define BOX_FORCE_OFF  3


/************************* MQTT *********************************/

#define MQTT_SERVER  "192.168.1.27"
#define MQTT_PORT 1883

//Constants
const char TV_STATE_OFF = 1;
const char TV_STATE_ON  = 2;


const char TV_FORCE_DISABLE = '1';
const char TV_FORCE_OFF     = '2';

int boxStatus = BOX_OFF;
unsigned long lastSwitch = 0;


void processTvStateMsg(char* topic, byte* payload, unsigned int length)
{

  Serial.print("Checking if state topic");
  if(String(topic) == "TV/force"){
	  Serial.println("Received TV/force change message");
		 Serial.println("changing TV relay state");
	     if ((char)payload[0] == TV_FORCE_DISABLE) {
		    Serial.println("TV FORCE DISABLED received");
			boxStatus = BOX_OFF;			
	     }
	     else if ((char)payload[0] == TV_FORCE_OFF){
		    Serial.println("TV FORCE OFF received");
 		    boxStatus = BOX_FORCE_OFF;
			switchAllOff();
			
	     }
		 else {
		    Serial.print("Unknown payload : ");
			Serial.println((char)payload[0]);
		 }

  }
}

void setup() {
  Serial.begin(115200);
  pinMode(BOX_STATUS_PIN, INPUT);
  pinMode(AMPLIFIER_RELAY_PIN, OUTPUT);
  digitalWrite(AMPLIFIER_RELAY_PIN, HIGH);
  pinMode(TV_RELAY_PIN, OUTPUT);
  digitalWrite(TV_RELAY_PIN, HIGH);  
  
  delay(10);
  
  myMqtt = new MqttConnection(SENSOR_ID, WLAN_SSID, WLAN_PASS, MQTT_SERVER, MQTT_PORT);
  myMqtt->registerCustomProcessing(&processTvStateMsg);
  myMqtt->addSubscription("force");

}

bool checkToggleTooFast(){
  Serial.print("now :");
  Serial.println(millis());
  Serial.print("last :");
  Serial.println(lastSwitch);  
   if(millis() - lastSwitch < DELAY_BETWEEN_TOGGLE)
      return true;
   else
      return false;
      
}
void switchAllOn(){
  if(checkToggleTooFast()){
    Serial.println("Skipping switch because toggling too fast");
    return;
  }
    
  Serial.println("Switching all ON");
  digitalWrite(AMPLIFIER_RELAY_PIN, LOW);
  digitalWrite(TV_RELAY_PIN, LOW);  
  boxStatus = BOX_ON;
  myMqtt->publishValue("state", TV_STATE_ON, 1);  
  lastSwitch = millis();

}
void switchAllOff(){
  if(checkToggleTooFast()){
    Serial.println("Skipping switch because toggling too fast");
    return;
  }
  Serial.println("Switching all OFF");
  digitalWrite(AMPLIFIER_RELAY_PIN, HIGH);
  digitalWrite(TV_RELAY_PIN, HIGH);  
  if(boxStatus != BOX_FORCE_OFF)
    boxStatus = BOX_OFF;
  myMqtt->publishValue("state", TV_STATE_OFF, 1);  
  lastSwitch = millis();
}


void loop() {
  // put your main code here, to run repeatedly:

  int boxStatusPin = digitalRead(BOX_STATUS_PIN);
  if (!myMqtt->connected()) {
    myMqtt->reconnect();
  }
  myMqtt->loop();

  if((boxStatusPin == 1) and (boxStatus == BOX_OFF)){
        switchAllOn();
  }
  if((boxStatusPin == 0) and (boxStatus == BOX_ON)){
        switchAllOff();
  }
  
  
  delay(DELAY_BETWEEN_BOX_CHECK);


}
