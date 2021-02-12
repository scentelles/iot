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
door01Pin = 1 #(must be pull down at reset) 
door02Pin = 2 #(must be pull down at reset) 
door03Pin = 3 #(must be pull down at reset) 
door04Pin = 4 #(must be pull down at reset) 
door05Pin = 5 #(must be pull down at reset) 
door06Pin = 6 #(must be pull down at reset) 
door07Pin = 7 #(must be pull down at reset) 
door08Pin = 8 #(must be pull down at reset) 
door09Pin = 9 #(must be pull down at reset) 
door10Pin = 10 #(must be pull down at reset) 
door11Pin = 11 #(must be pull down at reset) 
door12Pin = 12 #(must be pull down at reset) 
door13Pin = 13 #(must be pull down at reset) 
door14Pin = 14 #(must be pull down at reset) 
door15Pin = 15 #(must be pull down at reset) 
door16Pin = 16 #(must be pull down at reset) 
door17Pin = 17 #(must be pull down at reset) 
door18Pin = 18 #(must be pull down at reset) 
door19Pin = 19 #(must be pull down at reset) 




GPIO.setmode(GPIO.BCM)

# Pin Setup:
for x in range (1,20):
  GPIO.setup(x, GPIO.OUT) # LED pin set as output


for x in range (1,20):
  print("toggling " )
  print(x)
  GPIO.output(x, GPIO.HIGH)
  time.sleep(1)
  GPIO.output(x, GPIO.LOW)
  time.sleep(1)
  GPIO.output(x, GPIO.HIGH)
  time.sleep(1)
  GPIO.output(x, GPIO.LOW)
  time.sleep(1)

