#include <ArduinoOTA.h>

#include <RemoteDebug.h> //https://github.com/JoaoLopesF/RemoteDebug v2.1.2


#include "MqttConnection.h" 
#include <PZEM004Tv30.h> //https://github.com/mandulaj/PZEM-004T-v30


MqttConnection * myMqtt;


/************************* WiFi Access Point *********************************/
#define WLAN_SSID       "SFR_34A8"
#define WLAN_PASS       "ab4ingrograstanstorc"

#define SENSOR_ID "NRJMON"

#define LOOP_DELAY 15 * 1000

/************************* MQTT *********************************/

#define MQTT_SERVER  "192.168.1.27"
#define MQTT_PORT 1883

/*************************
 * 
 *  ESP32 initialization
 * ---------------------
 * 
 * The ESP32 HW Serial interface can be routed to any GPIO pin 
 * Here we initialize the PZEM on Serial2 
 */
#define PZEM_RX_PIN 13 // connected to TX pin on PZEM
#define PZEM_TX_PIN 15 // connected to RX pin on PZEM

#define PZEM_SERIAL Serial2
#define CONSOLE_SERIAL Serial
PZEM004Tv30 pzem1(PZEM_SERIAL, PZEM_RX_PIN, PZEM_TX_PIN, 1);
PZEM004Tv30 pzem2(PZEM_SERIAL, PZEM_RX_PIN, PZEM_TX_PIN, 2);
PZEM004Tv30 pzem3(PZEM_SERIAL, PZEM_RX_PIN, PZEM_TX_PIN, 3);
PZEM004Tv30 pzem4(PZEM_SERIAL, PZEM_RX_PIN, PZEM_TX_PIN, 4);



//==================================================
RemoteDebug Debug;



void debugPrintln(const char * msg)
{
#ifdef TELNET_DEBUG    
    Debug.println(F(msg));
#else
    Serial.println(msg);
#endif
}
void debugPrint(const char * msg)
{
#ifdef TELNET_DEBUG    
    Debug.print(F(msg));
#else
    Serial.print(msg);
#endif
}
//====================================================

  


//============================

void processNRJMONMsg(char* topic, byte* payload, unsigned int length)
{
 
 //debugPrintln("Received custom command");
 String strPayload = "";
 for (int i = 0; i < length; i++)
 {
     strPayload += (char)payload[i];
 }
 int intPayload = atoi(strPayload.c_str());


  if(String(topic) == "NRJMON/PING")
  {
      debugPrintln("Ping received.replying");
      myMqtt->publishValue("NRJMON/PONG", "1");
     
  }

  else if(String(topic) == "NRJMON/RESET")
  {
      debugPrintln("reset received");
      pzem1.resetEnergy();
      pzem2.resetEnergy();
      pzem3.resetEnergy();
      pzem4.resetEnergy();     
  }

    
}



void initOTA()
{
  
  ArduinoOTA.setHostname("ESP_32_NRJMON");

  ArduinoOTA
    .onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH)
        type = "sketch";
      else // U_SPIFFS
        type = "filesystem";

      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
      debugPrintln(String("Start updating " + type).c_str());
    })
    .onEnd([]() {
      debugPrintln("\nEnd");
    })
    .onProgress([](unsigned int progress, unsigned int total) {
      debugPrintln(String("Progress: " +  (progress / (total / 100))).c_str());
    })
    .onError([](ota_error_t error) {
      debugPrintln(String("Error: " + error).c_str());
      if (error == OTA_AUTH_ERROR) debugPrintln("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) debugPrintln("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) debugPrintln("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) debugPrintln("Receive Failed");
      else if (error == OTA_END_ERROR) debugPrintln("End Failed");
    });

  ArduinoOTA.begin();

}


void setup() {
    // Debugging Serial port
    CONSOLE_SERIAL.begin(115200);



    myMqtt = new MqttConnection(SENSOR_ID, WLAN_SSID, WLAN_PASS, MQTT_SERVER, MQTT_PORT);
    myMqtt->registerCustomProcessing(&processNRJMONMsg);
    myMqtt->addSubscription("NRJMON/PING");
    myMqtt->addSubscription("NRJMON/RESET");
    
    initOTA();

    // init remote debug
    Debug.begin("ESP32");  
    
}

void getAllValues(PZEM004Tv30 &pzem, String pzemId)
{
    // Read the data from the sensor

    float voltage = pzem.voltage();
    float current = pzem.current();
    float power = pzem.power();
    float energy = pzem.energy();
    float frequency = pzem.frequency();
    float pf = pzem.pf();

    // Check if the data is valid
    if(isnan(voltage)){
        CONSOLE_SERIAL.println("Error reading voltage");
    } else if (isnan(current)) {
        CONSOLE_SERIAL.println("Error reading current");
    } else if (isnan(power)) {
        CONSOLE_SERIAL.println("Error reading power");
    } else if (isnan(energy)) {
        CONSOLE_SERIAL.println("Error reading energy");
    } else if (isnan(frequency)) {
        CONSOLE_SERIAL.println("Error reading frequency");
    } else if (isnan(pf)) {
        CONSOLE_SERIAL.println("Error reading power factor");
    } else {

        // Print the values to the Serial console
        CONSOLE_SERIAL.print("Voltage: ");      CONSOLE_SERIAL.print(voltage);      CONSOLE_SERIAL.println("V");
        String tempTopic = pzemId + String("/VOLTAGE");
        myMqtt->publishValue(tempTopic.c_str(), String(voltage).c_str());
        
        CONSOLE_SERIAL.print("Current: ");      CONSOLE_SERIAL.print(current);      CONSOLE_SERIAL.println("A");
        CONSOLE_SERIAL.print("Power: ");        CONSOLE_SERIAL.print(power);        CONSOLE_SERIAL.println("W");
        tempTopic = pzemId + String("/POWER");
        myMqtt->publishValue(tempTopic.c_str(), String(power).c_str());
        CONSOLE_SERIAL.print("Energy: ");       CONSOLE_SERIAL.print(energy,3);     CONSOLE_SERIAL.println("kWh");
        tempTopic = pzemId + String("/ENERGY");
        myMqtt->publishValue(tempTopic.c_str(), String(energy).c_str());
 
        CONSOLE_SERIAL.print("Frequency: ");    CONSOLE_SERIAL.print(frequency, 1); CONSOLE_SERIAL.println("Hz");
        CONSOLE_SERIAL.print("PF: ");           CONSOLE_SERIAL.println(pf);

    }
}


void loop() {
    ArduinoOTA.handle();
    Debug.handle();
    if (!myMqtt->connected()) {
      debugPrintln("MQTT RECONNECT!!!!!!!!!!!!!!!!!!!!!");
      myMqtt->reconnect();
    }
    
    myMqtt->loop();
    CONSOLE_SERIAL.println("Reading all values of PZEM1");
    getAllValues(pzem1, "PZEM1");
    CONSOLE_SERIAL.println("Reading all values of PZEM2");
    getAllValues(pzem2, "PZEM2");
    CONSOLE_SERIAL.println("Reading all values of PZEM3");
    getAllValues(pzem3, "PZEM3");
    CONSOLE_SERIAL.println("Reading all values of PZEM4");
    getAllValues(pzem4, "PZEM4");
    

    CONSOLE_SERIAL.println();
    delay(LOOP_DELAY);
}
