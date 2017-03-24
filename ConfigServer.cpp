#include "ConfigServer.h"




static ConfigServer * confServer;



void ConfigServer::returnOK()
{
  sendHeader("Connection", "close");
  sendHeader("Access-Control-Allow-Origin", "*");
  send(200, "text/plain", "Configuration updated. Rebooting in 10 seconds...\r\n");
  delay (10000);
  ESP.restart();
}

void ConfigServer::returnFail(String msg)
{
  sendHeader("Connection", "close");
  sendHeader("Access-Control-Allow-Origin", "*");
  send(500, "text/plain", msg + "\r\n");
}


void ConfigServer::writeConfigField(int index, String str){
    
    eepromConfig.stageWriteField(index, str);
}

void ConfigServer::handleRoot()
{
   if (confServer->args() > 1){
      Serial.println("Field update requested");
      String sensorName = confServer->arg("SENSOR_NAME");
      String wifiName   = confServer->arg("WIFI_NAME");
      String wifiPwd    = confServer->arg("WIFI_PWD");
      String mqttIp     = confServer->arg("MQTT_IP");
      String mqttPort   = confServer->arg("MQTT_PORT");

      Serial.print("sensorName :");
      Serial.println(sensorName);
      Serial.print("wifiName :");
      Serial.println(wifiName);
      Serial.print("wifiPwd :");
      Serial.println(wifiPwd);
      Serial.print("mqttIp :");
      Serial.println(mqttIp);
      Serial.print("mqttPort :");
      Serial.println(mqttPort);

      confServer->writeConfigField(0, sensorName);
      confServer->writeConfigField(1, wifiName);
      confServer->writeConfigField(2, wifiPwd);
      confServer->writeConfigField(3, mqttIp);
      confServer->writeConfigField(4, mqttPort);

      confServer->eepromConfig.commit();
      confServer->returnOK();

  }
  else {
    Serial.println("no  arg");
    confServer->send(200, "text/html", confServer->getHtmlPage().c_str());
  }

}


String ConfigServer::getHtmlPage(){
   String result =
"<!DOCTYPE HTML>"
"<html>"
"<head>"
"<meta name = \"viewport\" content = \"width = device-width, initial-scale = 1.0, maximum-scale = 1.0, user-scalable=0\">"
"<title>ESP8266 Web Form Demo</title>"
"<style>"
"\"body { background-color: #808080; font-family: Arial, Helvetica, Sans-Serif; Color: #000000; }\""
"</style>"
"</head>"
"<body>"
"<h1>Device Config</h1>"
"<FORM action=\"/\" method=\"post\">"
"<P>"
"Sensor name : "
"<INPUT type=\"text\" name=\"SENSOR_NAME\" value=\"" + config.sensorName + "\"<BR>"
"WIFI access point :"
"<INPUT type=\"text\" name=\"WIFI_NAME\" value=\"" + config.wifiName + "\"<BR>"
"WIFI password :"
"<INPUT type=\"text\" name=\"WIFI_PWD\" value=\"" + config.wifiPwd + "\"<BR>"
"MQTT broker IP address : "
"<INPUT type=\"text\" name=\"MQTT_IP\" value=\"" + config.mqttIp + "\"<BR>"
"MQTT port : "
"<INPUT type=\"text\" name=\"MQTT_PORT\" value=\"" + config.mqttPort + "\"<BR>"
"<INPUT type=\"submit\" value=\"Send\"> <INPUT type=\"reset\">"
"</P>"
"</FORM>"
"</body>"
"</html>";

   return result;
    
}

    
    ConfigServer::ConfigServer(IPAddress ip) : ESP8266WebServer(DEFAULT_PORT){
        serverIP = ip;
        //Ugly but not other solution found... 
        //Would need to pass context to callback regitrations in ESP8266WebServer API
        confServer = this;
        eepromConfig.readAll();
        config.sensorName = eepromConfig.readField(0);
        config.wifiName   = eepromConfig.readField(1);
        config.wifiPwd    = eepromConfig.readField(2);
        config.mqttIp     = eepromConfig.readField(3);
        config.mqttPort   = eepromConfig.readField(4);  
               
       
    }


    MqttConfig ConfigServer::getMqttConfig(){
        return config;
        
    }
    
    void ConfigServer::start(){
       
        Serial.println("starting config server");
        WiFi.mode(WIFI_AP);
        WiFi.softAPConfig(serverIP, serverIP, IPAddress(255, 255, 255, 0));
        int result = WiFi.softAP("ESP_CONFIG");
        if(result == true)
        {
            Serial.println("Config server ready");
        }
        else
        {
            Serial.println("Config server failed to start!");
        }
          
    
        on("/", &ConfigServer::handleRoot);
        //on("/", fn_wrapper);
        
        begin();
   }
  
