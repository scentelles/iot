#if defined(ESP8266)
  #include <ESP8266WebServer.h>
#else
  #include <WebServer.h>
#endif

#include "PersistentConfig.h"


#define DEFAULT_PORT 80

typedef struct MqttConfig{
    String sensorName;
    String wifiName;
    String wifiPwd;
    String mqttIp;
    String mqttPort;    
} MqttConfig;
#if defined(ESP8266)  
class ConfigServer : public ESP8266WebServer{
#else
class ConfigServer : public WebServer{
#endif
    public :
    ConfigServer(IPAddress ip);
    void start();
    void writeConfigField(int index, String str);
    MqttConfig getMqttConfig();
    
    private :
    IPAddress serverIP;
#if defined(ESP8266) 
    ESP8266WebServer * server;
#else
    WebServer * server;
#endif

    PersistentConfig eepromConfig;
    MqttConfig config;  
   
        
    void returnOK();
    void returnFail(String msg);
    static void handleRoot();
    String getHtmlPage();
  
};



