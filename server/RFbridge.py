import http.client, urllib.request, urllib.parse, urllib.error
import requests
import time
import os
import urllib.request, urllib.error, urllib.parse
import socket

import paho.mqtt.client as mqtt

#import RPi.GPIO as GPIO
import sys

import smtplib

MQTT_IP_ADDRESS = "192.168.1.27"

#SWITCH 1 (Door)
#home/OpenMQTTGateway/SRFBtoMQTT 3151714


#SWITCH 2 (Deck)
#home/OpenMQTTGateway/SRFBtoMQTT 10867362


#Remote 1(Dorian) A  
#home/OpenMQTTGateway/SRFBtoMQTT 6454993
#Remote 1(Dorian) B
#home/OpenMQTTGateway/SRFBtoMQTT 6454994

#Remote 2(Gael) A 
#home/OpenMQTTGateway/SRFBtoMQTT 14993777
#Remote 2(Gael) B 
#home/OpenMQTTGateway/SRFBtoMQTT 14993778


#move/detected
#home/OpenMQTTGateway/SRFBtoMQTT 14786398

#Door bell
#home/OpenMQTTGateway/SRFBtoMQTT 16276098


#button bell 2
#home/OpenMQTTGateway/SRFBtoMQTT 4462722

deck_state = 0



class MyException(Exception):
    pass



try:
    server = smtplib.SMTP('smtp.gmail.com', 587)
    server.ehlo()
except:
    print('Something went wrong')


def getImageFromCamera1():
    if os.path.exists("/home/pi/camera1.jpg"):
        os.remove("/home/pi/camera1.jpg")
    #url = "http://192.168.2.122:554/snapshot"
    #url = "http://192.168.2.80:554/snapshot"
    url = "http://192.168.2.29/snap.jpg?usr=admin&pwd=admin"
    try:
        r = requests.get(url, verify = False, timeout=2)
        open("/home/pi/camera1.jpg", 'w+b').write(r.content)
    except (requests.exceptions.Timeout, requests.exceptions.ConnectionError) as error:
        print("Time out! or connection error :")
        print(error)
        os.system('cp /home/pi/camera_disconnected.jpg /home/pi/camera1.jpg')
	
def getImageFromCamera2():
    if os.path.exists("/home/pi/camera2.jpg"):
        os.remove("/home/pi/camera2.jpg")
    url = "http://192.168.2.13/snap.jpg?usr=admin&pwd=admin"
    try:
        r = requests.get(url, verify = False, timeout=2)
        open("/home/pi/camera2.jpg", 'w+b').write(r.content)
    except (requests.exceptions.Timeout, requests.exceptions.ConnectionError) as error:
        print("Time out! or connection error :")
        print(error)
        os.system('cp /home/pi/camera_disconnected.jpg /home/pi/camera2.jpg')


# The callback for when the client receives a CONNACK response from the server.
def on_connect(client, userdata, flags, rc):
    print(("Connected with result code "+str(rc)))

    # Subscribing in on_connect() means that if we lose the connection and
    # reconnect then subscriptions will be renewed.

    client.subscribe("home/OpenMQTTGateway/SRFBtoMQTT")
    
# The callback for when a PUBLISH message is received from the server.
def on_message(client, userdata, msg):
    global deck_state
    print((msg.topic+" "+str(msg.payload)+"\n"))
#    if msg.topic == "Door/open":
#        print msg.topic
    if msg.payload == b'3151714':
        print("Switch 1 trigger received")
        print("trigger external door")
        client.publish("Door/open", payload='2', qos=1, retain=False)

#Dorian
    if msg.payload == b'6454993':
        print("Remote 1 A trigger received")
        print("trigger external door")
        client.publish("Door/open", payload='12', qos=1, retain=False)
	#getImageFromCamera2();
        getImageFromCamera1();
        time.sleep(1)
        client.publish("Door/dorian", payload='Portail Dorian', qos=1, retain=False)

    if msg.payload == b'6454994':
        print("Remote 1 B trigger received")
        print("trigger external door")
        client.publish("Door/open", payload='2', qos=1, retain=False)
        getImageFromCamera1();
        time.sleep(1)
        client.publish("Door/dorian", payload='Portail Dorian', qos=1, retain=False)

#Elisa
    if msg.payload == b'16736113':
        print("Remote 3 A trigger received")
        print("trigger external door")
        client.publish("Door/open", payload='12', qos=1, retain=False)
	#getImageFromCamera2();
        getImageFromCamera1();
        time.sleep(1)
        client.publish("Door/dorian", payload='Portail Elisa', qos=1, retain=False)

    if msg.payload == b'16736114':
        print("Remote 1 C trigger received")
        print("trigger external door")
        client.publish("Door/open", payload='2', qos=1, retain=False)
        getImageFromCamera1();
        time.sleep(1)
        client.publish("Door/dorian", payload='Portail Elisa', qos=1, retain=False)
	
#Gael
    if msg.payload == b'14993777':
        print("Remote 2 A trigger received")
        print("trigger external door")
        client.publish("Door/open", payload='12', qos=1, retain=False)
	#getImageFromCamera2();
        getImageFromCamera1();
        time.sleep(1)
        client.publish("Door/gael", payload='Portail Gael', qos=1, retain=False)

    if msg.payload == b'14993778':
        print("Remote 2 B trigger received")
        print("trigger external door")
        client.publish("Door/open", payload='2', qos=1, retain=False)
        time.sleep(1)
        getImageFromCamera1();
        client.publish("Door/gael", payload='Portail Gael', qos=1, retain=False)

    if msg.payload == b'16276098':
        print("Door bell trigger received")
        print("trigger Ring")
        getImageFromCamera1();
        time.sleep(1)
        client.publish("Door/bell", payload='1', qos=1, retain=False)
		
    if msg.payload == b'14786398':
        print("Move trigger received")
        print("trigger move")
        client.publish("move/detected", payload='1', qos=1, retain=False)




    if msg.payload == b'10867362':
        print("Switch 2 trigger received")
        print("trigger Deck light")
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

client.connect(MQTT_IP_ADDRESS)

# Blocking call that processes network traffic, dispatches callbacks and
# handles reconnecting.
# Other loop*() functions are available that give a threaded interface and a
# manual interface.
client.loop_forever()
