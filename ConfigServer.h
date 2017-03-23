#include <ESP8266WebServer.h>

#include "PersistentConfig.h"


#define DEFAULT_PORT 80

typedef struct MqttConfig{
    String sensorName;
    String wifiName;
    String wifiPwd;
    String mqttIp;
    String mqttPort;    
} MqttConfig;

class ConfigServer : public ESP8266WebServer{
    public :
    ConfigServer(IPAddress ip);
    void start();
    void writeConfigField(int index, String str);
    MqttConfig getMqttConfig();
    
    private :
    IPAddress serverIP;
    ESP8266WebServer * server;
    PersistentConfig eepromConfig;
    MqttConfig config;  
   
        
    void returnOK();
    void returnFail(String msg);
    static void handleRoot();
    String getHtmlPage();
  
};



