#include <PZEM004T.h>

#include "MqttConnection.h"
MqttConnection * myMqtt;



/************************* WiFi Access Point *********************************/

#define WLAN_SSID       "SFR_34A8"
#define WLAN_PASS       "ab4ingrograstanstorc"

/************************* MQTT *********************************/

#define MQTT_SERVER  "192.168.1.27"
#define MQTT_PORT 1883

#define SENSOR_ID "EDF"

#define PROBE_DELAY 10000  //10 seconds
#define ENERGY_VS_POWER_PROBE_RATIO 6

PZEM004T pzem(D2,D1);
IPAddress ip(192,168,1,1);

int count = 0;

void setup() {
  Serial.begin(115200);
  pzem.setAddress(ip);
  myMqtt = new MqttConnection(SENSOR_ID, WLAN_SSID, WLAN_PASS, MQTT_SERVER, MQTT_PORT);

}

void loop() {

  if (!myMqtt->connected()) {
    myMqtt->reconnect();
  }
  myMqtt->loop();

  count ++;
  
/*  float v = pzem.voltage(ip);
  wdt_reset();
  if (v < 0.0) v = 0.0;
  Serial.print(v);Serial.print("V; ");
  myMqtt->publishValue("voltage", (int)v, 1);
*/

/*  float i = pzem.current(ip);
  wdt_reset();
  if(i >= 0.0){ 
    Serial.print(i);Serial.print("A; "); 
    myMqtt->publishValue("current", i, 1);
  }
*/

  if (count % ENERGY_VS_POWER_PROBE_RATIO != 0)
  {
	  float p = pzem.power(ip);
	  wdt_reset();
	  if(p >= 0.0){ 
		Serial.print(p);Serial.print("W; ");
		myMqtt->publishValue("power", p, 1);
	  }
  }
  
  else
  {
	  float e = pzem.energy(ip);
	  wdt_reset();
	  if(e >= 0.0){ 
		Serial.print(e);Serial.print("Wh; "); 
		myMqtt->publishValue("energy", e, 1);
	  }
  }
  
  Serial.println();

  delay(PROBE_DELAY);
}


