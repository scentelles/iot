//Libraries
#include <DHT.h>

#include "MqttConnection.h"
MqttConnection * myMqtt;

//Constants
#define SENSOR_ID "ROOM1_SENSOR"
#define PROBE_TEMPO 10000
#define DHTPIN D4     // what pin we're connected to
#define DHTTYPE DHT11   // DHT 11

#define RELAY_PIN D2


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
}


void loop()
{
  if (!myMqtt->connected()) {
    myMqtt->reconnect();
  }
  myMqtt->loop();

  long now = millis(); 
  if (now - lastMsg > PROBE_TEMPO) {
      lastMsg = now;
      
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
  }

}

   
