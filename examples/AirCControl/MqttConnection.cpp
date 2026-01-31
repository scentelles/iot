#include "MqttConnection.h"
#include <stdarg.h>


char tmpChars[32];
  
PubSubClient * g_mqttClient;
void (* customMsgProcessing)(char* topic, byte* payload, unsigned int length) = NULL;

static void serialLogf(const char *fmt, ...)
{
  if (!Serial) {
    return;
  }
  if (Serial.availableForWrite() < 32) {
    return;
  }
  char buf[192];
  va_list args;
  va_start(args, fmt);
  vsnprintf(buf, sizeof(buf), fmt, args);
  va_end(args);
  Serial.print(buf);
}

void  mycallback(char* topic, byte* payload, unsigned int length) {
  serialLogf("Message arrived [%s] %.*s\n", topic, length, (char *)payload);

  if(String(topic) == "PING_LEAF_TOPIC"){
	  if ((char)payload[0] == '1') {
		serialLogf("Ping received\n");
		String tmp_string = topic;
		tmp_string += " : I'm alive!!!!";
		
		g_mqttClient->publish("connection_events", tmp_string.c_str());
		serialLogf("after publish\n");
	  } else {
		serialLogf("doing nothing\n");
	  }
  }
  else{
	  if(customMsgProcessing)
	  {
		  customMsgProcessing(topic, payload, length);
		  
	  }
  }
  

}



/****************************** Wifi connect function ***************************************/
void MqttConnection::wifiSetup(const char* ssid, const char* pass) {

  delay(10);
  // We start by connecting to a WiFi network
  serialLogf("\nConnecting to %s\n", ssid);
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);

  const unsigned long connectTimeoutMs = 20000;
  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < connectTimeoutMs) {
    delay(1000);
    serialLogf(".");
  }

  randomSeed(micros());

  if (WiFi.status() == WL_CONNECTED) {
    serialLogf("\nWiFi connected\nIP address: %s\n", WiFi.localIP().toString().c_str());
  }

  WiFi.setAutoReconnect(true);
  WiFi.persistent(true);


}

void MqttConnection::registerCustomProcessing(void (*myFunc)(char* topic, byte* payload, unsigned int length) )
{
	customMsgProcessing = myFunc;

}

void MqttConnection::publishValue(const char * leafTopic, const char* msg)
{
    serialLogf("Publish message: %s\n", msg);
    
    
    String outTopic = sensorId_ + "/";
    outTopic += leafTopic;
    //Serial.print("on topic: ");
    //Serial.println(outTopic.c_str());
    publish(outTopic.c_str(), msg);
}
void MqttConnection::publishValue(const char * leafTopic, float value, int precision)
{
    char msg[50];
    String  format = "\%." + precision;
    format += "f";

    
    snprintf (msg, 75, "%.*f", precision, value);
    //String tmp = value;
    serialLogf("Publish message: %s\n", msg);
    
    
    String outTopic = sensorId_ + "/";
    outTopic += leafTopic;
    //Serial.print("on topic: ");
    //Serial.println(outTopic.c_str());
    publish(outTopic.c_str(), msg);
}

void MqttConnection::subscribeAll()
{
	for (int i = 0; i < nbLeafTopic_; i++) {
	  char* tmpLeafTopic = leafTopicList_[i];

      String tmp_string = sensorId_;
	  tmp_string += "/";
      tmp_string += tmpLeafTopic;
	  serialLogf("Subscribing to : %s\n", tmp_string.c_str());
	  subscribe(tmp_string.c_str());		
	}
	
}

bool MqttConnection::reconnect() {
  static unsigned long lastAttempt = 0;
	
	//first, check Wifi is connected
  //while (WiFi.status() != WL_CONNECTED) {
   // Serial.print('.');
    //delay(1000);
 // }
  
  // Loop until we're reconnected
  if (connected()) {
    return false;
  }

  unsigned long now = millis();
  if (now - lastAttempt < 5000) {
    return false;
  }
  lastAttempt = now;

  serialLogf("Attempting MQTT connection...");
  // Create a random client ID
  String clientId = sensorId_;

  // Attempt to connect
  if (connect(clientId.c_str())) {
    serialLogf("connected\n");
    // Once connected, publish an announcement...
    String connectMsg = "New connection from " + clientId;
    publish("connection_events", connectMsg.c_str());
    // ... and resubscribe
    subscribeAll();
  } 
  else {
    serialLogf("failed, rc=%d try again in 5 seconds\n", state());
  }
  return true;
}

void MqttConnection::addSubscription(const char * leafTopic)
{
	if(nbLeafTopic_ < MAX_SUBSCRIBE_LEAF_TOPIC - 1)
	{
	    nbLeafTopic_++;
	    strcpy(leafTopicList_[nbLeafTopic_ - 1], leafTopic); 
	}
	else
	{
		serialLogf("Error : too many topics to subscribe\n");
	}
}

MqttConnection::MqttConnection(const char* sensorId, const char* ssid, const char* pass, const char* mqttServer, int mqttPort) : PubSubClient(wifiClient_)
{
  addSubscription(PING_LEAF_TOPIC);
  sensorId_ = sensorId; 
  wifiSetup(ssid, pass);


  //make a copy in a global char array due to bug somewhere in the pubsub lib?
  strcpy(tmpChars, mqttServer);
  setServer(tmpChars, mqttPort);
  setCallback(mycallback);
  setKeepAlive(60);
  g_mqttClient = this;
  

}


