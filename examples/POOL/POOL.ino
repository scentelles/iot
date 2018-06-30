//Libraries

#include "MqttConnection.h"
MqttConnection * myMqtt;

#include <OneWire.h>
#include <DallasTemperature.h>

//Constants
#define SENSOR_ID "POOL"
#define LOOP_DELAY 50

#define PROBE_TEMPO 60000  //1 measure per minute
#define ONE_WIRE_BUS D1  // DS18B20 pin
#define RELAY_PIN D8
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature DS18B20(&oneWire);

#define SensorPin A0            //pH meter Analog output to ESP8266 Analog Input
#define Offset -2.7            //deviation compensate
#define ArrayLenth  40    //times of collection
float pHArray[ArrayLenth];   //Store the average value of the sensor feedback
   

//Variables

#define RELAY_STATE_OFF '1'
#define RELAY_STATE_ON '2'


long lastProbe = 0;


/************************* WiFi Access Point *********************************/

#define WLAN_SSID       "SFR_34A8"
#define WLAN_PASS       "ab4ingrograstanstorc"

/************************* MQTT *********************************/

#define MQTT_SERVER  "192.168.1.27"
#define MQTT_PORT 1883


/*************************** Sketch Code ************************************/



void processCommandMsg(char* topic, byte* payload, unsigned int length)
{
  String commandTopic = String(SENSOR_ID) + "/" + "command";
  Serial.print("Checking if topic:#");
  Serial.print(commandTopic);
  Serial.println("#");

  if(String(topic) == commandTopic){
     Serial.println("Received state topic change");
       if ((char)payload[0] == RELAY_STATE_ON) {
          Serial.println("RELAY STATE ON received");

          digitalWrite(RELAY_PIN, HIGH);
          char tmpChar = RELAY_STATE_ON;
          myMqtt->publishValue("state", &tmpChar);
       }
       else if ((char)payload[0] == RELAY_STATE_OFF){
        Serial.println("RELAY STATE OFF received");

          digitalWrite(RELAY_PIN, LOW);    
          char tmpChar = RELAY_STATE_OFF;
          myMqtt->publishValue("state", &tmpChar); 
       }
     else {
        Serial.print("Unknown payload : ");
      Serial.println((char)payload[0]);
     }

  }
}



void setup()
{
  Serial.begin(115200);
  myMqtt = new MqttConnection(SENSOR_ID, WLAN_SSID, WLAN_PASS, MQTT_SERVER, MQTT_PORT);

  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);
  
  myMqtt->registerCustomProcessing(&processCommandMsg);
  myMqtt->addSubscription("command");
  
}


float avergearray(float* arr, int number){
 int i;
 int max,min;
 float avg;
 float amount=0;
 if(number<=0){
   Serial.println("Error number for the array to avraging!/n");
   return 0;
 }
 
   for(i=0;i<number;i++){
       amount+=arr[i]; 
   }
   avg = amount/(number);

 return avg;
}

// Return RSSI or 0 if target SSID not found
int32_t getRSSI(String target_ssid) {
  byte available_networks = WiFi.scanNetworks();

  for (int network = 0; network < available_networks; network++) {
    if (WiFi.SSID(network) == target_ssid) {
      return WiFi.RSSI(network);
    }
  }
  return 0;
}


void loop()
{
  if (!myMqtt->connected()) {
    myMqtt->reconnect();
  }
  myMqtt->loop();

  long now = millis(); 
  float pHValue,voltage, voltage1;
  if (now - lastProbe > PROBE_TEMPO)
  {
	  Serial.println("Probig NOW");
	  lastProbe = now;
	  DS18B20.requestTemperatures(); 
	  float temperature = DS18B20.getTempCByIndex(0);
	  Serial.print("Temperature: ");
	  Serial.println(temperature);
	  if (temperature == 85.0 || temperature == (-127.0)) {
		  Serial.println("Failed to read from sensor!");
	  }
	  else
	  {
		  //send to MQTT server
		  myMqtt->publishValue("temperature", temperature, 1);    
	  }
	  

	     for(int i = 0; i < ArrayLenth; i++)
	     {
		 voltage1=analogRead(SensorPin);
		 voltage = voltage1*5.0/1024;
		 pHArray[i] = 3.5*voltage+Offset;
		 }
		 pHValue = avergearray(pHArray, ArrayLenth);

		Serial.print("    Voltage : ");
	  Serial.println(voltage1,3); 
	  Serial.print("    pH value: ");
	  Serial.println(pHValue,2);
	   myMqtt->publishValue("ph", pHValue, 1);  
	   

	  /*int32_t rssi = getRSSI(WLAN_SSID);
	  Serial.print("\nWifi strength : ");
	  Serial.println(rssi);*/
  }
  

    delay(LOOP_DELAY);
}

   



