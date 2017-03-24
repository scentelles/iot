


#include <IRremoteESP8266.h>
#include "MqttConnection.h"



MqttConnection * myMqtt;

#define SENSOR_ID "IR"

/************************* WiFi Access Point *********************************/

#define WLAN_SSID       "SFR_34A8"
#define WLAN_PASS       "ab4ingrograstanstorc"
#define MQTT_SERVER  "192.168.1.27"
#define MQTT_PORT 1883

int khz = 38; // 38kHz carrier frequency for both NEC and Samsung
#define SEND_PIN D5
IRsend irsend(SEND_PIN); //an IR led is connected to GPIO4 (pin D2 on NodeMCU)
decode_results results;


//#############
//# Receive
#define RECV_PIN D2
IRrecv irrecv(RECV_PIN);

 
unsigned int irTVpwr[] = {2500,550,1300,550,700,550,1300,550,700,550,1300,550,700,550,650,550,1300,550,700,550,700,550,700,550,700}; 

unsigned int climAllume[] = {3400,1850,500,1200,500,300,500,300,500,300,500,1200,500,300,500,300,500,300,500,1200,500,300,500,300,500,300,500,1200,500,300,500,300,500,300,500,1200,500,300,500,350,450,350,450,1250,500,300,500,300,500,300,500,1200,500,300,500,300,500,300,500,1200,500,300,500,300,500,350,500,1250,500,300,500,300,500,300,500,1200,500,350,500,300,500,300,500,300,500,300,500,1200,500,300,500,300,500,1250,500,300,500,350,500};

unsigned int climEteinte[] = {3400,1850,500,1250,500,300,500,350,450,350,450,1250,500,300,500,300,500,350,500,1250,500,300,500,300,500,300,500,1250,500,350,450,350,450,350,500,1250,500,300,500,350,450,350,450,1250,500,300,500,300,500,350,500,1250,500,300,500,300,500,300,500,1250,500,350,450,350,450,350,500,1250,500,350,450,350,450,350,450,1250,500,300,500,350,450,350,500,350,450,350,450,350,450,350,450,350,450,1250,500,350,450,350,500};
void processIrCmdMsg(char* topic, byte* payload, unsigned int length)
{
    
    // Serial.println("sending toggle power");
    // irsend.sendRaw(irTVpwr, sizeof(irTVpwr)/ sizeof(irTVpwr[0]), 38);
    // delay(20);
    //irsend.sendRaw(irTVpwr, sizeof(irTVpwr)/ sizeof(irTVpwr[0]), 38);
    // delay(20);
    // irsend.sendRaw(irTVpwr, sizeof(irTVpwr)/ sizeof(irTVpwr[0]), 38);
    if ((char)payload[0] == '2') {
		    Serial.println("Clim ON received");
           // irsend.sendRaw(climAllume, sizeof(climAllume)/ sizeof(climAllume[0]), 38);
            //delay(100);
            irsend.sendRaw(climAllume, sizeof(climAllume)/ sizeof(climAllume[0]), 38);
            delay(20);
            irsend.sendRaw(climAllume, sizeof(climAllume)/ sizeof(climAllume[0]), 38);
            delay(20);
            irsend.sendRaw(climAllume, sizeof(climAllume)/ sizeof(climAllume[0]), 38);
    }
    if ((char)payload[0] == '1') {
		    Serial.println("Clim OFF received");
           // irsend.sendRaw(climEteinte, sizeof(climEteinte)/ sizeof(climEteinte[0]), 38);
           // delay(100);
            irsend.sendRaw(climEteinte, sizeof(climEteinte)/ sizeof(climEteinte[0]), 38);
            delay(20);
            irsend.sendRaw(climEteinte, sizeof(climEteinte)/ sizeof(climEteinte[0]), 38);
            delay(20);
            irsend.sendRaw(climEteinte, sizeof(climEteinte)/ sizeof(climEteinte[0]), 38);
        
	}
}



void dump(decode_results *results) {
  // Dumps out the decode_results structure.
  // Call this after IRrecv::decode()
  int count = results->rawlen;
  if (results->decode_type == UNKNOWN) {
    Serial.print("Unknown encoding: ");
  }
  else if (results->decode_type == NEC) {
    Serial.print("Decoded NEC: ");

  }
  else if (results->decode_type == SONY) {
    Serial.print("Decoded SONY: ");
  }
  else if (results->decode_type == RC5) {
    Serial.print("Decoded RC5: ");
  }
  else if (results->decode_type == RC6) {
    Serial.print("Decoded RC6: ");
  }
  else if (results->decode_type == PANASONIC) {
    Serial.print("Decoded PANASONIC - Address: ");
    Serial.print(results->panasonicAddress, HEX);
    Serial.print(" Value: ");
  }
  else if (results->decode_type == LG) {
    Serial.print("Decoded LG: ");
  }
  else if (results->decode_type == JVC) {
    Serial.print("Decoded JVC: ");
  }
  else if (results->decode_type == AIWA_RC_T501) {
    Serial.print("Decoded AIWA RC T501: ");
  }
  else if (results->decode_type == WHYNTER) {
    Serial.print("Decoded Whynter: ");
  }
  Serial.print(results->value, HEX);
  Serial.print(" (");
  Serial.print(results->bits, DEC);
  Serial.println(" bits)");
  Serial.print("Raw (");
  Serial.print(count, DEC);
  Serial.print("): ");

  for (int i = 1; i < count; i++) {
    if (i & 1) {
      Serial.print(results->rawbuf[i]*USECPERTICK, DEC);
    }
    else {
      Serial.write('-');
      Serial.print((unsigned long) results->rawbuf[i]*USECPERTICK, DEC);
    }
    Serial.print(" ");
  }
  Serial.println();
}


void setup() {
  Serial.begin(115200);
  delay(10);
  irsend.begin();
  myMqtt = new MqttConnection(SENSOR_ID, WLAN_SSID, WLAN_PASS, MQTT_SERVER, MQTT_PORT);
  myMqtt->registerCustomProcessing(&processIrCmdMsg);
  myMqtt->addSubscription("CMD");
  myMqtt->addSubscription("CLIM");  
  irrecv.enableIRIn(); // Start the receiver
  

  
}

void loop() {

  if (!myMqtt->connected()) {
    myMqtt->reconnect();
  }
   myMqtt->loop();
   // irsend.sendPanasonic(0x8888, 0x88888824);
   // delay(2000);
    if (irrecv.decode(&results)) {
    Serial.println("Coucou Pipou j'ai recu un code de la telecommande :");
    Serial.println("Start decoding");
    Serial.println(results.value, HEX);
    dump(&results);
    irrecv.resume(); // Receive the next value
    }
    
  //  Serial.println("sending toggle power");
   // irsend.sendSony(0xa90, 12);
  //  irsend.sendRaw(irTVpwr, sizeof(irTVpwr)/ sizeof(irTVpwr[0]), 38);
  //  delay(20);
  //  irsend.sendRaw(irTVpwr, sizeof(irTVpwr)/ sizeof(irTVpwr[0]), 38);
  //  delay(20);
  //  irsend.sendRaw(irTVpwr, sizeof(irTVpwr)/ sizeof(irTVpwr[0]), 38);

    
    delay(100);
   // irsend.sendPanasonic(0x8888, 0x88888804);
   // delay(100);

    delay(200);  


}
