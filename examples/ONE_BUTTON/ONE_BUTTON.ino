
#include "MqttConnection.h"
#include "ConfigServer.h"


MqttConnection * myMqtt;
ConfigServer * myConfigServer;

IPAddress    configServerIP(10, 10, 10, 10);

int val; 
int PinCLK = D1; //4
int PinDT  = D2; //5
int PinSW  = D3; //0
static long encoderPos = -1;    // Au 1er démarrage, il passera à 0
int pinCLKLast = LOW;
int nbPas = 40;                 // Encoder resolution
int pinClkCurrent = LOW;

#define BOOT_MODE_CONFIG   1
#define BOOT_MODE_MQTT     2
#define BOOT_MODE_DEFAULT  3
int bootMode = BOOT_MODE_DEFAULT;

void setup() { 
    pinMode (PinCLK,INPUT);
    pinMode (PinDT,INPUT);
    pinMode (PinSW,INPUT);
    Serial.begin (115200);

    //Delay before checking button state and decide boot mode
    delay(3000);
    Serial.println("Starting device");
    myConfigServer = new ConfigServer(configServerIP);
    if(digitalRead(PinSW) == 0){
        Serial.println("Entering config boot mode");
        bootMode = BOOT_MODE_CONFIG;
        myConfigServer->start();
   }
   else{
       MqttConfig tmpConfig = myConfigServer->getMqttConfig();
       myMqtt = new MqttConnection(tmpConfig.sensorName.c_str(), tmpConfig.wifiName.c_str(), tmpConfig.wifiPwd.c_str(), tmpConfig.mqttIp.c_str(), tmpConfig.mqttPort.toInt());
   }
} 
 
void loop() { 
    if(bootMode == BOOT_MODE_CONFIG){
        myConfigServer->handleClient();
        delay(100);
    }
    else{
        if (!myMqtt->connected()) {
            Serial.println("Connecting to MQTT broker");
            myMqtt->reconnect();
            delay(5000);
        }
        myMqtt->loop();
      
        if (!(digitalRead(PinSW))) {      // Reset position upon button press 
            if(encoderPos != 0){
                encoderPos = 0;
                Serial.println("Reset position");
            }
       }
       
       pinClkCurrent = digitalRead(PinCLK);
       
       if (pinClkCurrent != pinCLKLast)  {  
         
           if (digitalRead(PinDT) == pinClkCurrent) {
               Serial.print("counter-clockwise, position ");
               encoderPos--;
               if ( encoderPos < 0 ) {
                   encoderPos = nbPas;
               }
           } 
           else {
               Serial.print("Clockwise, position ");
               encoderPos++;
               if ( encoderPos > ( nbPas - 1 ) ) {
                   encoderPos = 0;
               }
           }
           Serial.print (encoderPos); 
           myMqtt->publishValue("position", float(encoderPos), 1);

       } 
       pinCLKLast = pinClkCurrent;
    }


  
 }
