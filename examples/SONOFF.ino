#include <EEPROM.h>
#include "MqttConnection.h"
#include <ESP8266WebServer.h>


MqttConnection * myMqtt;
ESP8266WebServer server(80);

#define BOOT_MODE_CONFIG   1
#define BOOT_MODE_MQTT     2
#define BOOT_MODE_DEFAULT  3
#define RELAY_STATE_OFF '1'
#define RELAY_STATE_ON '2'

int bootMode = BOOT_MODE_DEFAULT;
bool statusGiven = false;
int pressCount = 0;
int currentRelayState = RELAY_STATE_OFF;

String sensorName;
char tmpS[32];

/************************* WiFi Access Point *********************************/

IPAddress    apIP(10, 10, 10, 10);

#define BUTTON 0
#define RELAY 12
#define LED 13
#define SPARE 14


#define EEPROM_FIELD_SIZE 32
#define EEPROM_NB_FIELD   5
char  eepromArray[EEPROM_NB_FIELD][EEPROM_FIELD_SIZE];


const char INDEX_RELAY_HTML[] =
"<!DOCTYPE HTML>"
"<html>"
"<head>"
"<meta name = \"viewport\" content = \"width = device-width, initial-scale = 1.0, maximum-scale = 1.0, user-scalable=0\">"
"<title>ESP8266 Web Form Demo</title>"
"<style>"
"\"body { background-color: #808080; font-family: Arial, Helvetica, Sans-Serif; Color: #000000; }\""
"</style>"
"</head>"
"<body>"
"<h1>RELAY Control</h1>"
"<FORM action=\"/\" method=\"post\">"
"<P>"
"RELAY<br>"
"<INPUT type=\"radio\" name=\"RELAY\" value=\"1\">On<BR>"
"<INPUT type=\"radio\" name=\"RELAY\" value=\"0\">Off<BR>"
"<INPUT type=\"submit\" value=\"Send\"> <INPUT type=\"reset\">"
"</P>"
"</FORM>"
"</body>"
"</html>";


const char INDEX_HTML[] =
"<!DOCTYPE HTML>"
"<html>"
"<head>"
"<meta name = \"viewport\" content = \"width = device-width, initial-scale = 1.0, maximum-scale = 1.0, user-scalable=0\">"
"<title>ESP8266 Web Form Demo</title>"
"<style>"
"\"body { background-color: #808080; font-family: Arial, Helvetica, Sans-Serif; Color: #000000; }\""
"</style>"
"</head>"
"<body>"
"<h1>ESP8266 Web Form Demo</h1>"
"<FORM action=\"/\" method=\"post\">"
"<P>"
"Sensor name : "
"<INPUT type=\"text\" name=\"SENSOR_NAME\" <BR>"
"WIFI access point :"
"<INPUT type=\"text\" name=\"WIFI_NAME\" <BR>"
"WIFI password :"
"<INPUT type=\"text\" name=\"WIFI_PWD\" <BR>"
"MQTT broker IP address : "
"<INPUT type=\"text\" name=\"MQTT_IP\" <BR>"
"MQTT port : "
"<INPUT type=\"text\" name=\"MQTT_PORT\" <BR>"
"<INPUT type=\"submit\" value=\"Send\"> <INPUT type=\"reset\">"
"</P>"
"</FORM>"
"</body>"
"</html>";


void returnOK()
{
  server.sendHeader("Connection", "close");
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "text/plain", "Configuration updated. Rebooting in 10 seconds...\r\n");
  delay (10000);
  ESP.restart();
}

void returnFail(String msg)
{
  server.sendHeader("Connection", "close");
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(500, "text/plain", msg + "\r\n");
}

void handleSubmitRelay()
{
  String relayValue;

  if (!server.hasArg("RELAY")) return returnFail("BAD ARGS");
  relayValue = server.arg("RELAY");
  if (relayValue == "1") {
    digitalWrite(LED, LOW);

    server.send(200, "text/html", INDEX_RELAY_HTML);
  }
  else if (relayValue == "0") {
    digitalWrite(LED, HIGH);
    server.send(200, "text/html", INDEX_RELAY_HTML);
  }
  else {
    returnFail("Bad RELAY value");
  }
}

void handleRootDefault()
{
  if (server.hasArg("RELAY")) {
    handleSubmitRelay();
  }
  else {
    server.send(200, "text/html", INDEX_RELAY_HTML);
  }
}



void handleRoot()
{
   if (server.args() > 1){
      Serial.println("Field update requested");
      sensorName = server.arg("SENSOR_NAME");
      String wifiName   = server.arg("WIFI_NAME");
      String wifiPwd    = server.arg("WIFI_PWD");
      String mqttIp     = server.arg("MQTT_IP");
      String mqttPort   = server.arg("MQTT_PORT");

      Serial.print("sensorName :");
      Serial.println(sensorName);
      Serial.print("wifiName :");
      Serial.println(wifiName);
      Serial.print("wifiPwd :");
      Serial.println(wifiPwd);
      Serial.print("mqttIp :");
      Serial.println(mqttIp);
      Serial.print("mqttPort :");
      Serial.println(mqttPort);

      strcpy(&eepromArray[0][0], sensorName.c_str());
      strcpy(&eepromArray[1][0], wifiName.c_str());
      strcpy(&eepromArray[2][0], wifiPwd.c_str());
      strcpy(&eepromArray[3][0], mqttIp.c_str());
      strcpy(&eepromArray[4][0], mqttPort.c_str());

      for (int addr = 0; addr < sizeof(eepromArray); addr++){
          EEPROM.write(addr, ((char*)eepromArray)[addr]);
      }
      EEPROM.commit();
      Serial.println("EEPROM updated");
      returnOK();

  }
  else {
    Serial.println("no  arg");
    server.send(200, "text/html", INDEX_HTML);
  }

}




void handleSubmit()
{
  String LEDvalue;

  if (!server.hasArg("LED")) return returnFail("BAD ARGS");
  LEDvalue = server.arg("LED");
  if (LEDvalue == "1") {

    server.send(200, "text/html", INDEX_HTML);
  }
  else if (LEDvalue == "0") {

    server.send(200, "text/html", INDEX_HTML);
  }
  else {
    returnFail("Bad LED value");
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

void blinkLedConfigMode(){
    blinkFast(1);
}

void deviceReadyBlink(){
    blinkSlow(3);
}

void deviceMqttReadyBlink(){
       blinkSlow(3);
       blinkFast(3);
       blinkSlow(3);
}


void processCommandMsg(char* topic, byte* payload, unsigned int length)
{

  Serial.print("Checking if state topic");
  String commandTopic = sensorName + "/" + "command";
  if(String(topic) == commandTopic){
     Serial.println("Received state topic change");
       if ((char)payload[0] == RELAY_STATE_ON) {
          Serial.println("RELAY STATE ON received");
          currentRelayState= RELAY_STATE_ON;

          digitalWrite(LED, HIGH);
          digitalWrite(RELAY, HIGH);
          char tmpChar = RELAY_STATE_ON;
          myMqtt->publishValue("state", &tmpChar);
       }
       else if ((char)payload[0] == RELAY_STATE_OFF){
        Serial.println("RELAY STATE OFF received");
          currentRelayState= RELAY_STATE_OFF;

          digitalWrite(LED, LOW);
          digitalWrite(RELAY, LOW);    
          char tmpChar = RELAY_STATE_OFF;
          myMqtt->publishValue("state", &tmpChar); 
       }
     else {
        Serial.print("Unknown payload : ");
      Serial.println((char)payload[0]);
     }

  }
}




void setup() {
  Serial.begin(115200);
  pinMode(BUTTON, INPUT);
  pinMode(RELAY, OUTPUT);
  pinMode(LED, OUTPUT);
  pinMode(SPARE, INPUT);

  EEPROM.begin(512);

  digitalWrite(LED, LOW);
  delay(3000);
  digitalWrite(LED, HIGH);

    for (int addr = 0; addr < sizeof(eepromArray); addr++){
          char tmpChar = (char)EEPROM.read(addr);
          ((char*)eepromArray)[addr] = tmpChar;
    }

      sensorName = eepromArray[0];
      String wifiName   = eepromArray[1];
      String wifiPwd    = eepromArray[2];
      String mqttIp     = eepromArray[3];
      String mqttPort   = eepromArray[4];

  //If button pressed at boot

  if(digitalRead(BUTTON) == 0){
        Serial.println("a bitch");
      bootMode = BOOT_MODE_CONFIG;
      Serial.println("starting config server");

      WiFi.mode(WIFI_AP);
      WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
      int result = WiFi.softAP("ESP_CONFIG");
      if(result == true)
      {
        Serial.println("Config server ready");
      }
      else
      {
        Serial.println("Config server failed to start!");
      }
          
    
      server.on("/", handleRoot);
      server.begin();

  }
  else{
      Serial.println("Starting with config :");
      Serial.print("\tObject Name :");
      Serial.println(sensorName);
      Serial.print("\tWifi Access point :");
      Serial.println(wifiName);
      Serial.print("\tWifi pwd :");
      Serial.println(wifiPwd);
      Serial.print("\tMQTT address :");
      Serial.println(mqttIp.c_str());
      Serial.print("\tMQTT port :");
      Serial.println(mqttPort);

      if(mqttPort != ""){
          bootMode = BOOT_MODE_MQTT;
         
          strcpy(tmpS, mqttIp.c_str());
          myMqtt = new MqttConnection(sensorName.c_str(), wifiName.c_str(), wifiPwd.c_str(), tmpS, mqttPort.toInt());
      
          myMqtt->registerCustomProcessing(&processCommandMsg);
          myMqtt->addSubscription("command");

          if(myMqtt->connected()){
              deviceMqttReadyBlink();
          }
          else
          {
            Serial.println("could not connect to MQTT server");
           }
      }
      else{
          WiFi.mode(WIFI_STA);
          WiFi.begin(wifiName.c_str(), wifiPwd.c_str());
         
          while (WiFi.status() != WL_CONNECTED) {
              delay(500);
              Serial.print(".");
           }
           Serial.println("");
           Serial.print("Connected to ");
           Serial.println(wifiName);
           Serial.print("IP address: ");
           Serial.println(WiFi.localIP());

            server.on("/", handleRootDefault);
            server.begin();
            
            Serial.print("Connect to http://xxxxxx.local or http://");
            Serial.println(WiFi.localIP());
            Serial.println("last number : ");
            int addressNumber = WiFi.localIP()[3];
            Serial.println(addressNumber);
            delay(1000);

            //Blink to indicate last digit of IP address
            int unit = addressNumber % 10;
            int dec = addressNumber / 10;
            blinkSlow(dec);
            delay(2000);
            blinkSlow(unit);
          


            
      }

 }

}

void toggleRelayState(){
    if(currentRelayState == RELAY_STATE_OFF)
    {
       currentRelayState= RELAY_STATE_ON;

       digitalWrite(LED, HIGH);
       digitalWrite(RELAY, HIGH);
       if(bootMode == BOOT_MODE_MQTT){
           char tmpChar = RELAY_STATE_ON;
           myMqtt->publishValue("state", &tmpChar);
       }
    }
    else
    {
       currentRelayState = RELAY_STATE_OFF;

       digitalWrite(LED, LOW);
       digitalWrite(RELAY, LOW);
       if(bootMode == BOOT_MODE_MQTT){
           char tmpChar = RELAY_STATE_OFF;
           myMqtt->publishValue("state", &tmpChar);
       }
    }


}


void loop() {
  // put your main code here, to run repeatedly:
 if(bootMode == BOOT_MODE_CONFIG){
    server.handleClient();
    blinkLedConfigMode();

}
else
{


  if(bootMode == BOOT_MODE_MQTT){
      if (!myMqtt->connected()) {
         myMqtt->reconnect();
         statusGiven = false;
      }
      if (myMqtt->connected() & !statusGiven) {
         deviceMqttReadyBlink();
         statusGiven = true;
      }
      myMqtt->loop();


  }
  else{
    server.handleClient();
    
  }

  //In all case monitor HW button
  if(digitalRead(BUTTON) == 0){
       pressCount +=1;
       Serial.print("count :");
       Serial.println(pressCount);
       if(pressCount == 2){
           Serial.println("Toggling state");
           toggleRelayState();
       }
  }
  else{
        pressCount = 0;
  }



}



delay(100);

}

