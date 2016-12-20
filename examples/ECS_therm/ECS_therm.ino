#include <ESP8266WiFi.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <PubSubClient.h>

//Constants
#define SENSOR_ID "ECS"
#define PROBE_TEMPO 60000
#define ONE_WIRE_BUS D4  // DS18B20 pin
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature DS18B20(&oneWire);

float oldTemp;


/************************* WiFi Access Point *********************************/

#define WLAN_SSID       "SFR_34A8"
#define WLAN_PASS       "ab4ingrograstanstorc"


/************************* MQTT *********************************/

const char* mqtt_server = "192.168.1.27";

/************ Global State (you don't need to change this!) ******************/

// Create an ESP8266 WiFiClient class to connect to the MQTT server.
WiFiClient espClient;
// or... use WiFiFlientSecure for SSL
//WiFiClientSecure client;

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
PubSubClient client(espClient);


long lastMsg = 0;
char msg[50];
int value = 0;


/****************************** Wifi connect function ***************************************/
void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WLAN_SSID);

  WiFi.begin(WLAN_SSID, WLAN_PASS);

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

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1') {
    Serial.println("Ping received");
    String tmp_string = SENSOR_ID;
    tmp_string += "says : I'm alive!!!!";
    
    client.publish("connection_events", tmp_string.c_str());
   
  } else {
    Serial.println("doing nothing");
   
  }

}


void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = SENSOR_ID;

    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      String connectMsg = "New connection from " + clientId;
      client.publish("connection_events", connectMsg.c_str());
      // ... and resubscribe
      String tmp_string = "ping";
      tmp_string += clientId;
      
      client.subscribe(tmp_string.c_str());
    } 
    else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}


void setup() {
  Serial.begin(115200);

  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  
  
      
}

void loop() {
  float temp;
    
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  long now = millis(); 

  if (now - lastMsg > PROBE_TEMPO) {
      lastMsg = now;
      
      //Read data and store it to variables hum and temp
      DS18B20.requestTemperatures(); 
      temp = DS18B20.getTempCByIndex(0);
      Serial.print("Temperature: ");
      Serial.println(temp);
      
      if (temp == 85.0 || temp == (-127.0)) {
          Serial.println("Failed to read from sensor!");
      }
      else
      {
 

          //send to MQTT server
          snprintf (msg, 75, "%.1f", temp);
          Serial.print("Publish message: ");
          Serial.println(msg);
          String outTopicPrefix = SENSOR_ID;
          String outTopicTemp = outTopicPrefix + "/temp";
          client.publish(outTopicTemp.c_str(), msg);
          
      }
  }


  

}

