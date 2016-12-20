#include <ESP8266WiFi.h>
#include <PubSubClient.h>

/****************************** Wifi connect function ***************************************/


class MqttConnection : public PubSubClient
{  
  public:
  String sensorId_;
  WiFiClient wifiClient_;
  MqttConnection(const char* sensorId, const char* ssid, const char* pass, const char* mqttServer, int mqttPort)  ;
  void reconnect();
  void publishValue(const char * leafTopic, float value, int precision);
  
  private:
  void wifiSetup(const char* ssid, const char* pass);
  //static void callback(char* topic, byte* payload, unsigned int length);
};
