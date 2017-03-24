//Libraries
#include <DHT.h>

#include "MqttConnection.h"
MqttConnection * myMqtt;

//Constants
#define SENSOR_ID "ROOM1_SENSOR"
#define SLEEP_TIME_S 60
#define DHTPIN D4     // what pin we're connected to
#define DHTTYPE AM2301   

#define DHTPIN D5 
#define AIRPIN D4 
#define AIRPIN_ANALOG A0 

DHT dht(DHTPIN, DHTTYPE); //// Initialize DHT sensor



//Variables


float hum;  //Stores humidity value
float temp; //Stores temperature value


/************************* WiFi Access Point *********************************/

#define WLAN_SSID       "SFR_34A8"
#define WLAN_PASS       "ab4ingrograstanstorc"

/************************* MQTT *********************************/

#define MQTT_SERVER  "192.168.1.27"
#define MQTT_PORT 1883

/************ Global State (you don't need to change this!) ******************/

long lastMsg = 0;

/*************************** Sketch Code ************************************/


void setup()
{
  Serial.begin(115200);
  myMqtt = new MqttConnection(SENSOR_ID, WLAN_SSID, WLAN_PASS, MQTT_SERVER, MQTT_PORT);
  dht.begin();

  pinMode(DHTPIN, INPUT);  
  pinMode(AIRPIN, INPUT);
  pinMode(AIRPIN_ANALOG, INPUT);
  
}


void loop()
{
  if (!myMqtt->connected()) {
    myMqtt->reconnect();
  }
  myMqtt->loop();


 
  int analogSensor = analogRead(AIRPIN_ANALOG);
  Serial.print("air Quality : ");
  Serial.println(analogSensor);
  myMqtt->publishValue("quality", analogSensor, 1);
  
   int digitalSensor = digitalRead(AIRPIN);
  Serial.print("air Quality Digital : ");
  Serial.println(digitalSensor);     
  
  //Read data and store it to variables hum and temp
  hum  = dht.readHumidity();
  temp = dht.readTemperature();
  
  if (isnan(hum) || isnan(temp)) {
      Serial.println("Failed to read from DHT sensor!");
  }
  else
  {
      //Print temp and humidity values to serial monitor
      Serial.print("Humidity: ");
      Serial.print(hum);
      Serial.print(" %, Temp: ");
      Serial.print(temp);
      Serial.println(" Celsius");

      //send to MQTT server
      myMqtt->publishValue("temp", temp, 1);
      myMqtt->publishValue("hum", hum, 1);        
  }
  //wait a bit before going to deep sleep
  delay(1000);
  ESP.deepSleep(SLEEP_TIME_S * 1000000);
}

   
