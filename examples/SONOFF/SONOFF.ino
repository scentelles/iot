#include "MqttConnection.h"
#include "ConfigServer.h"
#include <ESP8266WebServer.h>


MqttConnection * myMqtt;
ConfigServer * myConfigServer;

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

String sensorNameForCallback;
char tmpS[32];

/************************* WiFi Access Point *********************************/


IPAddress    configServerIP(10, 10, 10, 10);

#define BUTTON 0
#define RELAY 12
#define LED 13
#define SPARE 14



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
  String commandTopic = sensorNameForCallback + "/" + "command";
  Serial.print("Checking if topic:#");
  Serial.print(commandTopic);
  Serial.println("#");

  if(String(topic) == commandTopic){
     Serial.println("Received state topic change");
       if ((char)payload[0] == RELAY_STATE_ON) {
          Serial.println("RELAY STATE ON received");
          currentRelayState= RELAY_STATE_ON;

          digitalWrite(LED, LOW);
          digitalWrite(RELAY, HIGH);
          char tmpChar = RELAY_STATE_ON;
          myMqtt->publishValue("state", &tmpChar);
       }
       else if ((char)payload[0] == RELAY_STATE_OFF){
        Serial.println("RELAY STATE OFF received");
          currentRelayState= RELAY_STATE_OFF;

          digitalWrite(LED, HIGH);
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

    digitalWrite(LED, LOW);
    delay(3000);
    digitalWrite(LED, HIGH);

    myConfigServer = new ConfigServer(configServerIP);

    //If button pressed at boot
    if(digitalRead(BUTTON) == 0){
        Serial.println("Entering config boot mode");
        bootMode = BOOT_MODE_CONFIG;
        myConfigServer->start();
    }
    else{
        MqttConfig tmpConfig = myConfigServer->getMqttConfig();
        sensorNameForCallback = tmpConfig.sensorName;
        if(tmpConfig.mqttPort != ""){
            WiFi.mode(WIFI_STA);
            WiFi.softAPdisconnect();

            
            bootMode = BOOT_MODE_MQTT;
            myMqtt = new MqttConnection(tmpConfig.sensorName.c_str(), tmpConfig.wifiName.c_str(), tmpConfig.wifiPwd.c_str(), tmpConfig.mqttIp.c_str(), tmpConfig.mqttPort.toInt());
      
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
            WiFi.softAPdisconnect();

            WiFi.begin(tmpConfig.wifiName.c_str(), tmpConfig.wifiPwd.c_str());
         
            while (WiFi.status() != WL_CONNECTED) {
                delay(500);
                Serial.print(".");
            }
            Serial.println("");
            Serial.print("Connected to ");
            Serial.println(tmpConfig.wifiName);
            Serial.print("IP address: ");
            Serial.println(WiFi.localIP());

            server.on("/", handleRootDefault);
            server.begin();
            
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
        myConfigServer->handleClient();
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
            //Only consider button pressed after 2 loops
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
