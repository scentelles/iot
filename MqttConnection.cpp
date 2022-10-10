#include "MqttConnection.h"


char tmpChars[32];
  
PubSubClient * g_mqttClient;
void (* customMsgProcessing)(char* topic, byte* payload, unsigned int length) = NULL;

void  mycallback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  if(String(topic) == "PING_LEAF_TOPIC"){
	  if ((char)payload[0] == '1') {
		Serial.println("Ping received");
		String tmp_string = topic;
		tmp_string += " : I'm alive!!!!";
		
		g_mqttClient->publish("connection_events", tmp_string.c_str());
		Serial.println("after publish");
	  } else {
		Serial.println("doing nothing");
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

  int nbTry = 0;
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
	nbTry++;
	if(nbTry > 20)
	{
      nbTry = 0;
      Serial.println("Too long to connect - Rebooting");
	  ESP.restart();
	}
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

}

void MqttConnection::registerCustomProcessing(void (*myFunc)(char* topic, byte* payload, unsigned int length) )
{
	customMsgProcessing = myFunc;

}

void MqttConnection::publishValue(const char * leafTopic, const char* msg)
{
   // Serial.print("Publish message: ");
   // Serial.println(msg);
    
    
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
    Serial.print("Publish message: ");
    Serial.println(msg);
    
    
    String outTopic = sensorId_ + "/";
    outTopic += leafTopic;
    //Serial.print("on topic: ");
   // Serial.println(outTopic.c_str());
    publish(outTopic.c_str(), msg);
}

void MqttConnection::subscribeAll()
{
	for (int i = 0; i < nbLeafTopic_; i++) {
	  char* tmpLeafTopic = leafTopicList_[i];

      String tmp_string = sensorId_;
	  tmp_string += "/";
      tmp_string += tmpLeafTopic;
	  Serial.print("Subscribing to : ");
	  Serial.println(tmp_string.c_str());
	  subscribe(tmp_string.c_str());		
	}
	
}

void MqttConnection::reconnect() {
  // Loop until we're reconnected
  while (!connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = sensorId_;

    // Attempt to connect
    if (connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      String connectMsg = "New connection from " + clientId;
      publish("connection_events", connectMsg.c_str());
      // ... and resubscribe
	  subscribeAll();

    } 
    else {
      Serial.print("failed, rc=");
      Serial.print(state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
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
		Serial.print("Error : too many topics to subscribe");
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
  g_mqttClient = this;
  

}


