import httplib, urllib
import time
import os
import urllib2
import socket

import paho.mqtt.client as mqtt

import RPi.GPIO as GPIO
import sys

import smtplib

#SWITCH 1 (Door)
#home/OpenMQTTGateway/SRFBtoMQTT 3151714


#SWITCH 2 (Deck)
#home/OpenMQTTGateway/SRFBtoMQTT 10867362


#Remote 1(Dorian) A  
#home/OpenMQTTGateway/SRFBtoMQTT 6454993

#move/detected
#home/OpenMQTTGateway/SRFBtoMQTT 14786398

deck_state = 0



class MyException(Exception):
    pass



try:
    server = smtplib.SMTP('smtp.gmail.com', 587)
    server.ehlo()
except:
    print 'Something went wrong'



# The callback for when the client receives a CONNACK response from the server.
def on_connect(client, userdata, flags, rc):
    print("Connected with result code "+str(rc))

    # Subscribing in on_connect() means that if we lose the connection and
    # reconnect then subscriptions will be renewed.

    client.subscribe("home/OpenMQTTGateway/SRFBtoMQTT")
    
# The callback for when a PUBLISH message is received from the server.
def on_message(client, userdata, msg):
    global deck_state
    print(msg.topic+" "+str(msg.payload)+"\n")
#    if msg.topic == "Door/open":
#        print msg.topic
    if msg.payload == "3151714":
	print "Switch 1 trigger received"
	print "trigger external door"
        client.publish("Door/open", payload='2', qos=1, retain=False)

    if msg.payload == "6454993":
	print "Remote 1 A trigger received"
	print "trigger external door"
        client.publish("Door/open", payload='2', qos=1, retain=False)
        client.publish("Door/dorian", payload='2', qos=1, retain=False)

    if msg.payload == "14786398":
	print "Move trigger received"
	print "trigger move"
        client.publish("move/detected", payload='1', qos=1, retain=False)

    if msg.payload == "10867362":
	print "Switch 2 trigger received"
	print "trigger Deck light"
	if(deck_state == 1):
	   newValue   = '1'
	   deck_state = 0
	else:
	   newValue   = '2'
	   deck_state = 1
	   	
        client.publish("DECK_LEDS/stairs", payload=newValue, qos=1, retain=False)
        client.publish("DECK/LIGHT/command", payload=newValue, qos=1, retain=False)


    
client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message

client.connect("localhost")

# Blocking call that processes network traffic, dispatches callbacks and
# handles reconnecting.
# Other loop*() functions are available that give a threaded interface and a
# manual interface.
client.loop_forever()
