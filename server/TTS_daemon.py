import paho.mqtt.client as mqtt
import os


# The callback for when the client receives a CONNACK response from the server.
def on_connect(client, userdata, flags, rc):
    print(("Connected with result code "+str(rc)))

    # Subscribing in on_connect() means that if we lose the connection and
    # reconnect then subscriptions will be renewed.

    client.subscribe("/speaker1/tts")
    client.subscribe("/speaker1/timer")

def speak(text):
    command = "pico2wave -w /var/local/wave.wav -l fr-FR '" + text + "' | lame - /home/pi/wave.mp3" 
    os.system(command) 
    client.publish("/slyzic/play", payload="http://192.168.1.27:1880/wave.mp3", qos=0, retain=True)


# The callback for when a PUBLISH message is received from the server.
def on_message(client, userdata, msg):

    print((msg.topic+" "+str(msg.payload)+"\n"))
    if msg.topic == "/speaker1/tts":
        text = msg.payload
        print(text)
        text = text.replace("'","''")
        print(text)
#generate the mp3 that will be served by node-red server
        command = "pico2wave -w /var/local/wave.wav -l fr-FR '" + text + "' | lame - /home/pi/wave.mp3" 
        #command = "echo toto"
        print(command)
        os.system(command) 
        
         #then route to proper audio speaker, trigering the proper MQTT topic
        client.publish("/slyzic/play", payload="http://192.168.1.27:1880/wave.mp3", qos=0, retain=True)
    
    if msg.topic == "/speaker1/timer":
        if(msg.payload == '1'):
            print("Timeout ")	  
            speak("Attention : temps ecoule")
        if(msg.payload == '2'):
            print("Starting Timer")
            speak("Demarrage du minuteur")
        if(msg.payload == '3'):
            print("Aborting Timer")
            speak("Arret du minuteur")
	    
client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message

client.connect("localhost")

# Blocking call that processes network traffic, dispatches callbacks and
# handles reconnecting.
# Other loop*() functions are available that give a threaded interface and a
# manual interface.
client.loop_forever()
