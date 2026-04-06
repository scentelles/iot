#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>

/****************************** Ethernet MQTT connection ***************************************/


#define PING_LEAF_TOPIC "ping"
#define MAX_SUBSCRIBE_LEAF_TOPIC 32
#define MAX_CHAR_TOPIC 32

class MqttConnection : public PubSubClient
{  
  public:
	  String sensorId_;
	  EthernetClient ethClient_;
	  MqttConnection(const char* sensorId, const char* mqttServer, int mqttPort);
	  bool reconnect();
      void publishValue(const char * leafTopic, const char* msg);
	  void publishValue(const char * leafTopic, float value, int precision);
      void addSubscription(const char * leafTopic);
	  void registerCustomProcessing(void (*)(char*, byte*, unsigned int) );
	  
  private:
	  char leafTopicList_[MAX_SUBSCRIBE_LEAF_TOPIC][MAX_CHAR_TOPIC];
	  int nbLeafTopic_ = 0;
	  void subscribeAll();
};
