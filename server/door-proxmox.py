#import http.client, urllib.request, urllib.parse, urllib.error
import os
os.environ['BLINKA_FT232H'] = "1"

import time
#import os
#import urllib.request, urllib.error, urllib.parse
#import socket

import paho.mqtt.client as mqtt

#import sys

import board
import digitalio




MQTT_IP_ADDRESS = "192.168.1.27"

door1Pin = digitalio.DigitalInOut(board.C0) #(must be pull down at reset) 
door1Pin.direction = digitalio.Direction.OUTPUT

door2Pin = digitalio.DigitalInOut(board.C1) #(must be pull down at reset) 
door2Pin.direction = digitalio.Direction.OUTPUT


# Initial state :
door1Pin.value = True
door2Pin.value = True

msg_at_boot = 1

PAYLOAD_DORIAN = b'24'
PAYLOAD_ELISA = b'8'

class MyException(Exception):
    pass



# The callback for when the client receives a CONNACK response from the server.
def on_connect(client, userdata, flags, rc):
    print(("Connected with result code "+str(rc)))

    # Subscribing in on_connect() means that if we lose the connection and
    # reconnect then subscriptions will be renewed.

    client.subscribe("Door/open")
    
# The callback for when a PUBLISH message is received from the server.
def on_message(client, userdata, msg):
    global msg_at_boot
    print((msg.topic+":"+str(msg.payload)+":\n"))
    if msg.topic == "Door/open":
        print(msg.topic)
        if(msg_at_boot == 1) :
           print("skipping first message at boot")
           msg_at_boot = 0
           return
        if msg.payload == PAYLOAD_ELISA:
           print("trigger Elisa door")
           door2Pin.value = False
           time.sleep(1)
           door2Pin.value = True
           client.publish("Door/elisa", payload=0, qos=0, retain=False)
        if msg.payload == PAYLOAD_DORIAN:
           print("trigger Elisa door")
           door2Pin.value = False
           time.sleep(1)
           door2Pin.value = True
           client.publish("Door/dorian", payload=0, qos=0, retain=False)


        if msg.payload == b'2':
           print("trigger external door 2")
           door1Pin.value = False
           time.sleep(1)
           door1Pin.value = True
        if msg.payload == b'3':
           print("trigger auto close external door")
           door1Pin.value = False
           time.sleep(1)
           door1Pin.value = True
           time.sleep(60)
           door1Pin.value = False
           time.sleep(1)
           door1Pin.value = True	
           client.publish("Door/open", payload=0, qos=0, retain=True)
        if msg.payload == b'12':
           print("trigger external door 12")
           door2Pin.value = False
           time.sleep(1)
           door2Pin.value = True
        if msg.payload == b'13':
           print("trigger auto close external door 13")
           door2Pin.value = False
           time.sleep(1)
           door2Pin.value = True
           time.sleep(60)
           door2Pin.value = False
           time.sleep(1)
           door2Pin.value = True    
           client.publish("Door/open", payload=0, qos=0, retain=True)

    
client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message

client.connect(MQTT_IP_ADDRESS)

# Blocking call that processes network traffic, dispatches callbacks and
# handles reconnecting.
# Other loop*() functions are available that give a threaded interface and a
# manual interface.
client.loop_forever()
