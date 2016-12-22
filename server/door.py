import httplib, urllib
import time
import os
import urllib2
import socket

import paho.mqtt.client as mqtt

import RPi.GPIO as GPIO
import sys



doorPin = 17 #(must be pull down at reset) 

GPIO.setmode(GPIO.BCM)

# Pin Setup:
GPIO.setup(doorPin, GPIO.OUT) # LED pin set as output

# Initial state for LEDs:
GPIO.output(doorPin, GPIO.HIGH)

class MyException(Exception):
    pass



# The callback for when the client receives a CONNACK response from the server.
def on_connect(client, userdata, flags, rc):
    print("Connected with result code "+str(rc))

    # Subscribing in on_connect() means that if we lose the connection and
    # reconnect then subscriptions will be renewed.

    client.subscribe("Door/open")
    
# The callback for when a PUBLISH message is received from the server.
def on_message(client, userdata, msg):
    print(msg.topic+" "+str(msg.payload)+"\n")
    if msg.topic == "Door/open":
        print msg.topic
	if msg.payload == "2":
	    print "trigger external door"
	    GPIO.output(doorPin, GPIO.LOW)
	    time.sleep(1)
	    GPIO.output(doorPin, GPIO.HIGH)
	if msg.payload == "3":
	    print "trigger auto close external door"
	    GPIO.output(doorPin, GPIO.LOW)
	    time.sleep(1)
	    GPIO.output(doorPin, GPIO.HIGH)
	    time.sleep(60)
	    GPIO.output(doorPin, GPIO.LOW)
	    time.sleep(1)
	    GPIO.output(doorPin, GPIO.HIGH)	
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
