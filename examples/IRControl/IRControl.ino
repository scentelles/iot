#include "MqttConnection.h"


#include <IRremoteESP8266.h>
//#include <ESP8266mDNS.h>


MqttConnection * myMqtt;

#define SENSOR_ID "IR"

/************************* WiFi Access Point *********************************/

#define WLAN_SSID       "SFR_34A8"
#define WLAN_PASS       "ab4ingrograstanstorc"
#define MQTT_SERVER  "192.168.1.27"
#define MQTT_PORT 1883

int khz = 38; // 38kHz carrier frequency for both NEC and Samsung

IRsend irsend(4); //an IR led is connected to GPIO4 (pin D2 on NodeMCU)

  // Insert RAW IR signal for "TV Power"
unsigned int irTVpwr[] = {4650,4250, 700,1550, 650,1550, 700,1550, 650,450, 650,500, 600,500, 600,500, 600,550, 550,1700, 550,1650, 600,1650, 550,550, 600,500, 600,550, 550,550, 600,500, 600,550, 550,1650, 600,550, 550,550, 600,500, 600,550, 550,550, 600,500, 600,1650, 600,500, 600,1650, 550,1700, 550,1650, 600,1650, 550,1650, 600,1650, 600};  // SAMSUNG E0E040BF

  // Insert RAW IR signal for "TV Source"
unsigned int irTVsrc[] = {4600,4300, 700,1550, 650,1550, 650,1600, 650,450, 650,450, 600,550, 550,550, 600,500, 600,1650, 550,1650, 600,1650, 550,550, 600,500, 600,550, 550,550, 550,550, 600,1650, 550,550, 550,550, 600,500, 600,500, 600,550, 550,550, 600,500, 600,550, 550,1650, 550,1700, 550,1650, 600,1600, 600,1650, 600,1600, 600,1650, 550};  // SAMSUNG E0E0807F


void processIrCmdMsg(char* topic, byte* payload, unsigned int length)
{

  Serial.print("Checking if state topic");
  Serial.println("Received IR command message");
  irsend.sendSony(0xa90, 12);
	
}


void setup() {
  Serial.begin(115200);
  delay(10);
  irsend.begin();
  myMqtt = new MqttConnection(SENSOR_ID, WLAN_SSID, WLAN_PASS, MQTT_SERVER, MQTT_PORT);
  myMqtt->registerCustomProcessing(&processIrCmdMsg);
  myMqtt->addSubscription("CMD");
  

  
}

void loop() {

  if (!myMqtt->connected()) {
    myMqtt->reconnect();
  }
  myMqtt->loop();
  Serial.println("Sony");

  delay(1000);
}
