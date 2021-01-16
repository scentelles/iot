import http.client, urllib.request, urllib.parse, urllib.error
import time
import os
import urllib.request, urllib.error, urllib.parse
import socket

import paho.mqtt.client as mqtt

import RPi.GPIO as GPIO
import sys



door1Pin = 17 #(must be pull down at reset) 
door2Pin = 18 #(must be pull down at reset) 


GPIO.setmode(GPIO.BCM)

# Pin Setup:
GPIO.setup(door1Pin, GPIO.OUT) # LED pin set as output
GPIO.setup(door2Pin, GPIO.OUT) # LED pin set as output

# Initial state for LEDs:
GPIO.output(door1Pin, GPIO.HIGH)
GPIO.output(door2Pin, GPIO.HIGH)

msg_at_boot = 1

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
        if msg.payload == b'2':
           print("trigger external door")
           GPIO.output(door1Pin, GPIO.LOW)
           time.sleep(0.5)
           GPIO.output(door1Pin, GPIO.HIGH)
        if msg.payload == b'3':
           print("trigger auto close external door")
           GPIO.output(door1Pin, GPIO.LOW)
           time.sleep(1)
           GPIO.output(door1Pin, GPIO.HIGH)
           time.sleep(60)
           GPIO.output(door1Pin, GPIO.LOW)
           time.sleep(1)
           GPIO.output(door1Pin, GPIO.HIGH)	
           client.publish("Door/open", payload=0, qos=0, retain=True)
        if msg.payload == b'12':
           print("trigger external door")
           GPIO.output(door2Pin, GPIO.LOW)
           time.sleep(1)
           GPIO.output(door2Pin, GPIO.HIGH)
        if msg.payload == b'13':
           print("trigger auto close external door")
           GPIO.output(door2Pin, GPIO.LOW)
           time.sleep(1)
           GPIO.output(door2Pin, GPIO.HIGH)
           time.sleep(60)
           GPIO.output(door2Pin, GPIO.LOW)
           time.sleep(1)
           GPIO.output(door2Pin, GPIO.HIGH)	
           client.publish("Door/open", payload=0, qos=0, retain=True)

    
client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message

client.connect("localhost")

# Blocking call that processes network traffic, dispatches callbacks and
# handles reconnecting.
# Other loop*() functions are available that give a threaded interface and a
# manual interface.
client.loop_forever()
