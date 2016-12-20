#include "MqttConnection.h"

PubSubClient * g_mqttClient;

void  mycallback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  if ((char)payload[0] == '1') {
    Serial.println("Ping received");
    String tmp_string = topic;
    tmp_string += " : I'm alive!!!!";
    
    g_mqttClient->publish("connection_events", tmp_string.c_str());
   
  } else {
    Serial.println("doing nothing");
   
  }

}



/****************************** Wifi connect function ***************************************/
void MqttConnection::wifiSetup(const char* ssid, const char* pass) {

   
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

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
    Serial.print("on topic: ");
    Serial.println(outTopic.c_str());
    publish(outTopic.c_str(), msg);
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
      String tmp_string = clientId;
      tmp_string += "/ping";
      subscribe(tmp_string.c_str());
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



MqttConnection::MqttConnection(const char* sensorId, const char* ssid, const char* pass, const char* mqttServer, int mqttPort) : PubSubClient(wifiClient_)
{
  sensorId_ = sensorId; 
  wifiSetup(ssid, pass);
      
 // mqttClient_ = new PubSubClient(*wifiClient_);
  setServer(mqttServer, mqttPort);
  setCallback(mycallback);
  g_mqttClient = this;
  

}


