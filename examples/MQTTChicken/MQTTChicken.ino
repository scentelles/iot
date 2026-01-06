#include "MqttConnection.h"
MqttConnection * myMqtt;


/************************* WiFi Access Point *********************************/

#define WLAN_SSID       "Wifi_garden"
#define WLAN_PASS       "060877040178"

#define SENSOR_ID "CHICKEN"

#define RELAY1 D1
#define RELAY2 D2

#define LED 13

#define LED_INVERTED 0

/************************* MQTT *********************************/

#define MQTT_SERVER  "192.168.1.27"
#define MQTT_PORT 1883

//Constants
const char CMD_UP_ON  =  '2';
const char CMD_UP_OFF =  '1';
const char CMD_DOWN_ON  =  '4';
const char CMD_DOWN_OFF =  '3';
#define RELAY_STATE_OFF '1'
#define RELAY_STATE_ON '2'

bool statusGiven = false;
int pressCount = 0;
int currentRelay1State = RELAY_STATE_OFF;
int currentRelay2State = RELAY_STATE_OFF;

void processStateMsg(char* topic, byte* payload, unsigned int length)
{

  Serial.print("Checking if state topic");
  if(String(topic) == "CHICKEN/cmd"){
	  Serial.println("Received Chicken change message");
       if ((char)payload[0] == CMD_UP_ON){
		    Serial.println("CHICKEN UP received");

			  switch1On();
        delay(200);
			  switch1Off();
	     }
       else 
       {
          if ((char)payload[0] == CMD_DOWN_ON){
       
          Serial.println("CHICKEN DOWN received");
  
          switch2On();
          delay(200);
          switch2Off();
          }
          else {
            Serial.print("Unknown payload : ");
            Serial.println((char)payload[0]);
          }
       }

  }

  if(String(topic) == "CHICKEN/ping"){
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
  pinMode(RELAY1, OUTPUT);
  pinMode(RELAY2, OUTPUT);
  pinMode(LED, OUTPUT);

  digitalWrite(RELAY1, HIGH);
  digitalWrite(RELAY2, HIGH);
  
  digitalWrite(LED, LOW);
  delay(3000);
  digitalWrite(LED, HIGH);

  
  delay(10);
  
  myMqtt = new MqttConnection(SENSOR_ID, WLAN_SSID, WLAN_PASS, MQTT_SERVER, MQTT_PORT);
  myMqtt->registerCustomProcessing(&processStateMsg);
  myMqtt->addSubscription("cmd");

}


void switch1On(){
    
  Serial.println("RELAY1 ON");
  digitalWrite(RELAY1, LOW);
  currentRelay1State= RELAY_STATE_ON;
  lightLed();
}
void switch1Off(){
    
  Serial.println("RELAY1 OFF");
  digitalWrite(RELAY1, HIGH);
  currentRelay1State= RELAY_STATE_OFF;
  shutLed();
}
void switch2On(){
    
  Serial.println("RELAY2 ON");
  digitalWrite(RELAY2, LOW);
  currentRelay2State= RELAY_STATE_ON;
  lightLed();
}
void switch2Off(){
    
  Serial.println("RELAY2 OFF");
  digitalWrite(RELAY2, HIGH);
  currentRelay2State= RELAY_STATE_OFF;
  shutLed();
}



void deviceMqttReadyBlink(){
       blinkSlow(3);
       blinkFast(3);
       blinkSlow(3);
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

  
  delay(100);


}
