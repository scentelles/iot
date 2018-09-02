
#include "MqttConnection.h"
MqttConnection * myMqtt;

//Constants
#define SENSOR_ID "DECK_LEDS"

/************************* WiFi Access Point *********************************/

#define WLAN_SSID       "SFR_34A8"
#define WLAN_PASS       "ab4ingrograstanstorc"

/************************* MQTT *********************************/

#define MQTT_SERVER  "192.168.1.27"
#define MQTT_PORT 1883



#define LED_PIN D4
#define PWM_PIN D8

void processCommandMsg(char* topic, byte* payload, unsigned int length)
{
  String commandTopic = String(SENSOR_ID) + "/" + "dimmer";
  Serial.print("Checking if topic:#");
  Serial.print(commandTopic);
  Serial.println("#");

  if(String(topic) == commandTopic){
     Serial.println("Received state topic change");
     int newValue = atoi((char*)payload);  
     if((newValue >= 0) && (newValue <=100)){
            Serial.print("Setting dimmer value ");
            Serial.println(newValue);
            analogWrite(PWM_PIN, 1024 - newValue*10);
            memset(payload,0, 8); //reset for next time.
     }
     else{
         Serial.print("Out of range value received : ");
         Serial.println(newValue);
     }
  }
  else
  {
    Serial.println("Received unknown topic");
  }
}


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW); 

  pinMode(PWM_PIN, OUTPUT);


  myMqtt = new MqttConnection(SENSOR_ID, WLAN_SSID, WLAN_PASS, MQTT_SERVER, MQTT_PORT);
  myMqtt->registerCustomProcessing(&processCommandMsg);
  myMqtt->addSubscription("dimmer");

  //we're now connected.
  digitalWrite(LED_PIN, HIGH); 
}

void loop() {
  if (!myMqtt->connected()) {
    myMqtt->reconnect();
  }
  myMqtt->loop();


  delay(10);
}
