#!/usr/bin/python3
from __future__ import print_function
import os


import datetime

import configparser
import time
#import pytz


from enum import Enum

import paho.mqtt.client as mqtt
from threading import Thread
from queue import Queue



heatMgrQueue = 0

ECS_COMMAND_OFF =  b'1'
ECS_COMMAND_ON  =  b'2'


lastTemperatureUpdate = datetime.datetime.now()
currentTemperatureUpdate = datetime.datetime.now()
global configWarningSender, configWarningRecipient, configSmtpLogin, configSmtpPassword
global ecsState, ecsRemoteState, ecsStateForced, ecsTemperature, ecsHeatTarget      

targetReached = False


#defines
ECS_HEAT_PROFILE_LOW         = "LOW"
ECS_HEAT_PROFILE_MEDIUM      = "MEDIUM"
ECS_HEAT_PROFILE_HIGH        = "HIGH"

ECS_STATE_OFF = "OFF"
ECS_STATE_ON  = "ON"

ECS_FORCE_DISABLED = "FORCE_DISABLED"
ECS_FORCE_OFF      = "FORCE_OFF"
ECS_FORCE_ON       = "FORCE_ON"


OVERHEAT_TEMPERATURE  = 61
UNDERHEAT_TEMPERATURE = 16



HEAT_MANAGER_PERIOD         = 1
DELAY_BETWEEN_TEMPERATURE_UPDATE = 60

class heatMgrMessage:
        def __init__(self, type, value, heatProfile="MEDIUM"):
            self.type  = type
            self.value = value
            self.heatProfile = heatProfile


#=========================================================================#                
#                      callback definition for MQTT                       #          
#=========================================================================#

# The callback for when the client receives a CONNACK response from the server.
def on_connect(client, userdata, flags, rc):
    print("Connected with result code "+str(rc) + "\n")
    client.subscribe("ECS/temp2")
    client.subscribe("ECS/force")
    client.subscribe("ECS/command")

def on_message(client, userdata, msg):
    tmpQueue = userdata
    tmpValue = ""
    #print ("Callback tmpQueue : ") 
    #print (tmpQueue)
    #print(msg.topic+" "+str(msg.payload) + "\n")
    if msg.topic == "ECS/force":
      #  print ("Callback sending force command to heatManager")
        if(msg.payload == b'0'):
            tmpValue = ECS_FORCE_DISABLED
        elif(msg.payload == b'1'):
            tmpValue = ECS_FORCE_OFF
        elif(msg.payload == b'2'):
            tmpValue = ECS_FORCE_ON
        else:
            print ("Callback Error : Unknown Force command :", msg.payload)
        tempMsg = heatMgrMessage('ECS_FORCE', tmpValue)
        tmpQueue.put(tempMsg)
    if msg.topic == "ECS/temp2":
      #  print("Callback sending temperature to heatManager")
        tempMsg = heatMgrMessage('ECS_TEMPERATURE', msg.payload)
        tmpQueue.put(tempMsg)
    if msg.topic == "ECS/command":
      #  print("Callback sending ECS Command to heatManager")
        tempMsg = heatMgrMessage('ECS_COMMAND', msg.payload)
        tmpQueue.put(tempMsg)



   
   
#=========================================================================#   
#           Heat profile to temperature conversion                        #
#=========================================================================#
        
def getTargetTemperature(profile):
    if (profile == ECS_HEAT_PROFILE_HIGH):
        return 62
    elif (profile == ECS_HEAT_PROFILE_MEDIUM):
        return 58
    elif (profile == ECS_HEAT_PROFILE_LOW):
        return 53
    else:
        print("Unknown heatprofile. Defaulting to 53", profile)
        return 53
        
        
def getStatusString():
    global ecsState, ecsRemoteState, ecsStateForced, ecsTemperature, ecsHeatTarget
    now = datetime.datetime.now().strftime('%Y-%m-%d %H:%M:%S')
    result  = "\n\n=====================\nTime : "
    result += now
    result += "\n\tECS state :" + ecsState
    result += "\n\tECS remote state :" + ecsRemoteState
    result += "\n\tECS force state :" + str(ecsStateForced)
    result += "\n\tECS temperature :" + str(ecsTemperature)
    result += "\n\tTarget temperature : " + str(ecsHeatTarget)
    result += "\n====================="
    return result


def warnMessage(msg, mqttClient):
    print(msg)  
    mqttClient.publish("ECS/warning", payload=msg, qos=1, retain=True)
    return

def checkTemperatureValidity(temperature, mqttClient):  
    if(temperature < UNDERHEAT_TEMPERATURE):
         if(warnSent == False):
            warnMessage("Warning, the temperature of the ECS is getting low. Consider forcing ON", mqttClient)
            warnSent = True
    if(temperature > OVERHEAT_TEMPERATURE):
        warnMessage("Warning, the temperature of the ECS is getting too high. Consider forcing OFF", mqttClient)


    
#=========================================================================#
# Heat manager thread body                                                #
# processes messages from Queue :                                         # 
#       - ECS force state (from MQTTLoop thread)                          #
#       - temperature(s)  (from MQTTLoop thread)                          #
#       - ECS command     (from MQTTLoop thread)                          #
#=========================================================================#
def heatManager(msqQueue, mqttClient):
    global ecsState, ecsRemoteState, ecsStateForced, ecsTemperature, ecsHeatTarget
    global lastTemperatureUpdate, currentTemperatureUpdate
    global targetReached
    ecsState       = ECS_STATE_OFF 
    ecsRemoteState = ECS_STATE_OFF 
    ecsStateForced = False
    ecsTemperature = 0
    ecsHeatTarget  = 0

    nbMsg = 0
    
    while True:
        #print ("HeatManager waiting for message")
        msg = msqQueue.get()
        msgType = msg.type
        msgValue = msg.value
        msgHeatProfile = msg.heatProfile

        #print ("HeatManager waking up. message received")
        nbMsg += 1
        #print ("nb message received")
        print ("#################################################")
        now = datetime.datetime.now().strftime('%Y-%m-%d %H:%M:%S')
        print (nbMsg, "Time : ", now)
	
      
            
        #process messages
        if(msgType == "ECS_COMMAND"):
            #Reset warning message status
            warnSent = False
            
            print(getStatusString())
            print("====================================")
            print("HeatManager : Processing ECS Command")
            if (msgValue == ECS_COMMAND_OFF):
                    print("Heat Manager : turning ECS OFF")
                    ecsState = ECS_STATE_OFF 
                    mqttClient.publish("ECS/state", payload='1', qos=1, retain=True)
                    mqttClient.publish("ECS/target", payload='0', qos=1, retain=True)

                    ecsRemoteState  = ECS_STATE_OFF
                    if(targetReached == False):
                        warnMessage("Temperature not reached before turning Off", mqttClient) 
	    
		    
            elif (msgValue == ECS_COMMAND_ON):
                    targetReached = False
                    ecsState = ECS_STATE_ON
                    ecsHeatTarget = getTargetTemperature(msgHeatProfile)
                    
                    #Check if temperature is recent enough to be valid, else warn user
                    currentTime = datetime.datetime.now()
                    deltaTime = currentTime - currentTemperatureUpdate
                    deltaTimeInSeconds = deltaTime.total_seconds()
                    if(deltaTimeInSeconds > DELAY_BETWEEN_TEMPERATURE_UPDATE *10):
                        message = "Warning : Switching ECS ON while temperature info may not be valid : \nsensor update exceeded 10 times maximum delta time: " + str(deltaTimeInSeconds) + "seconds"
                        warnMessage(message, mqttClient)  
                    
                    if(ecsTemperature < ecsHeatTarget):
                        print("Heat Manager : turning ECS ON")
                        mqttClient.publish("ECS/state", payload='2', qos=1, retain=True)
                        mqttClient.publish("ECS/target", payload=ecsHeatTarget, qos=1, retain=True)
			
                        ecsRemoteState  = ECS_STATE_ON
                    else:
                        message = "Heat Manager : No ECS ON despite command, due to target temperature already reached"                        
                        warnMessage(message, mqttClient)  

            else: 
                    print("Heat Manager : Error : unknown EcsCommand %s in received message" % msgValue)

        elif(msgType == "ECS_TEMPERATURE"):

            ecsTemperature = float(msgValue)

            lastTemperatureUpdate = currentTemperatureUpdate
            currentTemperatureUpdate = datetime.datetime.now()
            deltaTime = currentTemperatureUpdate - lastTemperatureUpdate
            deltaTimeInSeconds = deltaTime.total_seconds()
            
            if(deltaTimeInSeconds > DELAY_BETWEEN_TEMPERATURE_UPDATE * 4):
                message = "Warning : Temperature update from sensor exceeded 4 times maximum delta time: " + str(deltaTimeInSeconds) + "seconds"
                warnMessage(message, mqttClient)
            
            
            print ("updating temperature : ", ecsTemperature, " <> Target :", ecsHeatTarget)
            checkTemperatureValidity(ecsTemperature, mqttClient)
            #Check against temperature target when ECS is ON and not in forced mode
            if ((ecsState == ECS_STATE_ON) and (ecsStateForced == False)):
                if (ecsTemperature > ecsHeatTarget):
                    targetReached = True

                    print("Heat Manager : Switching ECS OFF due to target temperature reached")
                    ecsState = ECS_STATE_OFF
                    mqttClient.publish("ECS/state", payload='1', qos=1, retain=True)
                    mqttClient.publish("ECS/target", payload='0', qos=1, retain=True)

                    ecsRemoteState  = ECS_STATE_OFF
 
        elif(msgType == "ECS_FORCE"):   
            print(getStatusString())
            print ("HeatMgr : ecsState",  ecsState)
            if (msgValue == ECS_FORCE_OFF):
                print("Heat Manager : Forcing ECS OFF") 
                ecsStateForced = True
                print("\tHeat Manager : Switching ECS OFF") 
                mqttClient.publish("ECS/state", payload='1', qos=1, retain=True)
                mqttClient.publish("ECS/target", payload='0', qos=1, retain=True)

                ecsState        = ECS_STATE_OFF
                ecsRemoteState  = ECS_STATE_OFF
		
            elif (msgValue == ECS_FORCE_ON):
                print("Heat Manager : Forcing ECS ON") 
                ecsStateForced = True
                print("\tHeat Manager : Switching ECS ON") 
                mqttClient.publish("ECS/state", payload='2', qos=1, retain=True)
                mqttClient.publish("ECS/target", payload='100', qos=1, retain=True)

                ecsState = ECS_STATE_ON
                ecsRemoteState  = ECS_STATE_ON
            elif (msgValue == ECS_FORCE_DISABLED):
                print("Heat Manager : Disabling Forcing ECS") 
                ecsStateForced = False
                print("\tHeat Manager FORCE DISABLED : Switching ECS OFF") 
                mqttClient.publish("ECS/state", payload='1', qos=1, retain=True)
                mqttClient.publish("ECS/target", payload='0', qos=1, retain=True)
                ecsState        = ECS_STATE_OFF
                ecsRemoteState  = ECS_STATE_OFF


            else:  
                print("Heat Manager : Unknown message value %s " % msgValue)
        
        else:
            print("Heat Manager : Unknown message type %s " % msgType)




  
#=========================================================================#
# Main...                                                                 #
#=========================================================================#  
def main():
    print("STARTING main process")
    config = configparser.ConfigParser()
    config.read('myconf.conf')

    mqttAddress = config.get('MQTT', 'mqttAddress')
    mqttPort    = int(config.get('MQTT', 'mqttPort'))
    
    global heatMgrQueue
    heatMgrQueue = Queue()
    
    mqttClient = mqtt.Client(userdata=heatMgrQueue)
    print ("heatMgrQueue : ") 
    print (heatMgrQueue)
    mqttClient.on_connect = on_connect
    mqttClient.on_message = on_message
    mqttClient.connect(mqttAddress, mqttPort)
    mqttClient.loop_start()
    
    HeatManagerThread = Thread(target=heatManager, args=(heatMgrQueue,mqttClient,))    
    HeatManagerThread.start() 

    while 1:
        time.sleep(1000)

if __name__ == '__main__':
    main()
