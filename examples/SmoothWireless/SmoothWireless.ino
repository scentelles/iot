/*---------------------------------------------------------------------------------------------

  Open Sound Control (OSC) library for the ESP8266/ESP32

  Example for sending messages from the ESP8266/ESP32 to a remote computer
  The example is sending "hello, osc." to the address "/test".

  This example code is in the public domain.

--------------------------------------------------------------------------------------------- */
#if defined(ESP8266)
#include <ESP8266WiFi.h>
#else
#include <WiFi.h>
#endif
#include <WiFiUdp.h>
#include <OSCMessage.h>
#include <Wire.h>

#include "myADXL345.h"

char ssid[] = "xxx";                   // your network SSID (name)
char pass[] = "xxx";       // your network password

WiFiUDP Udp;                                // A UDP instance to let us send and receive packets over UDP
const IPAddress outIp(192,168,1,51);        // remote IP of your computer
const unsigned int outPort = 8000;          // remote port to receive OSC
const unsigned int localPort = 8888;        // local port to listen for OSC packets (actually not used for sending)

/* Assign a unique ID to this sensor at the same time */
Adafruit_ADXL345_Unified accel = Adafruit_ADXL345_Unified(12345);

void setup() {
    Serial.begin(115200);

    // Connect to WiFi network
    Serial.println();
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);
    WiFi.begin(ssid, pass);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");

    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());

    Serial.println("Starting UDP");
    Udp.begin(localPort);
    Serial.print("Local port: ");
    Serial.println(Udp.localPort());

    delay(1000); //wait for I2C init
    /* Initialise the sensor */
    if(!accel.begin())
    {
        /* There was a problem detecting the ADXL345 ... check your connections */
        Serial.println("Ooops, no ADXL345 detected ... Check your wiring!");
        while(1);  //will cause watchdog time out
    }

    /* Set the range to whatever is appropriate for your project */
    accel.setRange(ADXL345_RANGE_16_G);
    // displaySetRange(ADXL345_RANGE_8_G);
    // displaySetRange(ADXL345_RANGE_4_G);
    // displaySetRange(ADXL345_RANGE_2_G);
  
    /* Display some basic information on this sensor */
    displaySensorDetails(&accel);
  
    /* Display additional settings (outside the scope of sensor_t) */
    displayDataRate(&accel);
    displayRange(&accel);
    Serial.println("");

}


void sendOSCMessage(String msg, int parameter){
	OSCMessage OSCmsg(msg.c_str());
	if (parameter >= 0)
        OSCmsg.add(parameter);
    Udp.beginPacket(outIp, outPort);
    OSCmsg.send(Udp);
    Udp.endPacket();
    OSCmsg.empty();	
}


void sendNote(unsigned int note, unsigned int velocity, int duration)
{
    String prefix  =  "/vkb_midi/0/note/";
	String noteMsg = prefix + String(note);
	sendOSCMessage(noteMsg, velocity);
	if(duration >= 0)
	{
	    delay(duration);
	    sendOSCMessage(noteMsg, 0);
	}
}


void sendCC(unsigned int nb, unsigned int value)
{
    String prefix = "/vkb_midi/0/cc/";
	String ccMsg  = prefix + String(nb);
	sendOSCMessage(ccMsg, value);
}


void sendCommand(String command)
{
	String commandMsg  = "/" + command;
	sendOSCMessage(commandMsg, -1);
	
}

void loop() {

    sensors_event_t event; 

    Serial.println("sending first note");
	
	sendNote(45, 120, -1);
    
    while(1){

    	  accel.getEvent(&event);
        int newNote   = (int) (8 * event.acceleration.x) + 20;
        int newPitch  = (int) (8 * event.acceleration.y) + 20;
        int newFilter = (int) (10 * event.acceleration.x) + 20;

        sendCC(50, newPitch);
        sendCC(51, newFilter);
        sendNote(newNote, 120, 100);

        //displayAccelValues(&event);
      
    }
}
