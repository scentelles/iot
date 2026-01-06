#include "MqttConnection.h"
MqttConnection * myMqtt;


/************************* WiFi Access Point *********************************/

#define WLAN_SSID       "SFR_34A8"
#define WLAN_PASS       "ab4ingrograstanstorc"

#define SENSOR_ID "ECS"

 
#define BUTTON 0
#define RELAY D1

#define LED 13
#define SPARE 14

#define LED_INVERTED 0

/************************* MQTT *********************************/

#define MQTT_SERVER  "192.168.1.27"
#define MQTT_PORT 1883

//Constants
const char CMD_ON  =  '2';
const char CMD_OFF =  '1';
#define RELAY_STATE_OFF '1'
#define RELAY_STATE_ON '2'

bool statusGiven = false;
int pressCount = 0;
int currentRelayState = RELAY_STATE_OFF;

void processStateMsg(char* topic, byte* payload, unsigned int length)
{

  Serial.print("Checking if state topic");
  if(String(topic) == "ECS/cmd"){
	  Serial.println("Received Alarm change message");
       if ((char)payload[0] == CMD_ON){
		    Serial.println("ALARM ON received");

			  switchOn();
			
	     }
       else if ((char)payload[0] == CMD_OFF){
        Serial.println("ALARM OFF received");

        switchOff();
      
       }
		 else {
		    Serial.print("Unknown payload : ");
			Serial.println((char)payload[0]);
		 }

  }

  if(String(topic) == "ECS/ping"){
    Serial.println("Received ping");
    myMqtt->publishValue("pong", "Alive!");

  }

  
}
void blinkFast(int nbRepeat){
    for (int i = 0; i < nbRepeat; i++){
        digitalWrite(LED, LOW);
        delay(70);
        digitalWrite(LED, HIGH);
        delay(70);  
    }
}


void blinkSlow(int nbRepeat){
    for (int i = 0; i < nbRepeat; i++){
        digitalWrite(LED, LOW);
        delay(500);
        digitalWrite(LED, HIGH);
        delay(500);  
    }
}
void lightLed()
{
    if(LED_INVERTED)
        digitalWrite(LED, HIGH);
    else
        digitalWrite(LED, LOW);        
}
void shutLed()
{
    if(LED_INVERTED)
        digitalWrite(LED, LOW);
    else
        digitalWrite(LED, HIGH);
}

void setup() {
  Serial.begin(115200);
  pinMode(RELAY, OUTPUT);
  pinMode(LED, OUTPUT);
  pinMode(BUTTON, INPUT);

  digitalWrite(RELAY, HIGH);

  digitalWrite(LED, LOW);
  delay(3000);
  digitalWrite(LED, HIGH);

  
  delay(10);
  
  myMqtt = new MqttConnection(SENSOR_ID, WLAN_SSID, WLAN_PASS, MQTT_SERVER, MQTT_PORT);
  myMqtt->registerCustomProcessing(&processStateMsg);
  myMqtt->addSubscription("cmd");

}


void switchOn(){
    
  Serial.println("Switching Alarm ON");
  digitalWrite(RELAY, LOW);
  currentRelayState= RELAY_STATE_ON;
  lightLed();
}
void switchOff(){
    
  Serial.println("Switching Alarm OFF");
  digitalWrite(RELAY, HIGH);
  currentRelayState= RELAY_STATE_OFF;
  shutLed();
}
void deviceMqttReadyBlink(){
       blinkSlow(3);
       blinkFast(3);
       blinkSlow(3);
}

void toggleRelayState(){
    if(currentRelayState == RELAY_STATE_OFF)
    {
        switchOn();
    }
    else
    {
        switchOff();
    }
}





void loop() {
  // put your main code here, to run repeatedly:

  if (!myMqtt->connected()) {
    myMqtt->reconnect();
  }

  if (!myMqtt->connected()) {
      myMqtt->reconnect();
      statusGiven = false;
      }
  if (myMqtt->connected() & !statusGiven) {
      deviceMqttReadyBlink();
      shutLed();
      statusGiven = true;
     }  
     
  myMqtt->loop();

     //In all case monitor HW button
     if(digitalRead(BUTTON) == 0){
         pressCount +=1;
         //Only consider button pressed after 2 loops
         if(pressCount == 2){
             Serial.println("Toggling state");
             toggleRelayState();
         }
      }
      else{
         pressCount = 0;
      } 
  
  delay(100);


}
