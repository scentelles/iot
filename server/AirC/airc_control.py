import paho.mqtt.client as mqtt
import time

from threading import Thread

import json
from AirCManager import *
from AirCDefines import *
from Room import *
from AeroChannel import *

initDone = False
mqttClient = mqtt.Client()

masterChannel1 = AeroChannel(mqttClient, 0, "MASTER1") #useless
masterChannel2 = AeroChannel(mqttClient, 0, "MASTER2")
masterChannel3 = AeroChannel(mqttClient, 0, "MASTER3") #useless


roomList[CHAMBRE1] = Room(mqttClient, CHAMBRE1, 25, masterChannel2)
roomList[CHAMBRE2] = Room(mqttClient, CHAMBRE2, 25, masterChannel2)
roomList[CHAMBRE3] = Room(mqttClient, CHAMBRE3, 25, masterChannel2)
roomList[DREAMROOM] = Room(mqttClient, DREAMROOM, 35, masterChannel2)
roomList[SALON] = Room(mqttClient, SALON, 80, 0)
roomList[ETAGE] = Room(mqttClient, ETAGE, 35, 0)



myAirCManager = AirCManager(mqttClient, roomList)



# The callback for when the client receives a CONNACK response from the server.
def on_connect(client, userdata, flags, rc):
    print(("Connected with result code "+str(rc)))

    # Subscribing in on_connect() means that if we lose the connection and
    # reconnect then subscriptions will be renewed.
    for r in roomList:
        client.subscribe(MQTT_ADDRESS[r])
    for r in roomList:
        client.subscribe(MQTT_PREFIX + "/" + r + "/" + MQTT_SUFFIX_TARGETTEMP)   
    for r in roomList:
        client.subscribe(MQTT_PREFIX + "/" + r + "/" + MQTT_SUFFIX_AC_STATE)   
    
    client.subscribe(MQTT_ESP_AERAULIC_STATE)   
    client.subscribe(MQTT_ESP_INIT_DONE) 
    client.subscribe(MQTT_ESP_INIT_SERVO_DONE)     
    client.subscribe(MQTT_ESP_INIT_STARTED) 
    client.subscribe(MQTT_ESP_PONG)
    client.subscribe(MQTT_AC_MODE)
    
    
# The callback for when a PUBLISH message is received from the server.
def on_message(client, userdata, msg):

    print((msg.topic+":"+str(msg.payload)+":\n"))
    print (msg.payload)
    
    if(msg.topic.find(MQTT_SUFFIX_AC_STATE) != -1):
        print( "AC state change received")
        room = getRoomFromAddress(msg.topic)
        roomList[room].setAC_ON(msg.payload)
	
    elif(msg.topic.find(MQTT_SUFFIX_TARGETTEMP) != -1):
        print( "target temp change received")
        room = getRoomFromAddress(msg.topic)
        roomList[room].setTemperatureTarget(msg.payload)

    elif(msg.topic == MQTT_ESP_AERAULIC_STATE):
        if(msg.payload == AERO_CONFIG_ONGOING):
            print("Configuration ongoing - Toggle TBD")
        if(msg.payload == b'3'):
            myAirCManager.aeraulicState = AERO_CONFIGURED


    elif(msg.topic == MQTT_ESP_INIT_STARTED):
        if(myAirCManager.FSMState == STATE_READY):
            print("ESP32 reset detected\n")
            mqttClient.publish("AC/ERROR", "ESP RESET DETECTED!!!")
            myAirCManager.FSMState == STATE_INIT
	    
    elif(msg.topic == MQTT_ESP_INIT_DONE):
        if(myAirCManager.FSMState == STATE_WAIT_ESP_INIT):
            print("ESP32 connection OK\n")
            mqttClient.publish("AC/ERROR", "ESP CONNECTION OK")
            myAirCManager.FSMState = STATE_READY
        else:
            print("ERROR : INIT done received while Host not waiting for itESP32 connection OK\n")

    elif(msg.topic == MQTT_ESP_INIT_SERVO_DONE):
        if(myAirCManager.FSMState == STATE_WAIT_ESP_INIT):
            print("ESP32 connection OK\n")
            mqttClient.publish("AC/ERROR", "ESP CONNECTION OK")
            myAirCManager.FSMState = STATE_READY


    elif(msg.topic == MQTT_ESP_PONG):
        myAirCManager.pingAck = True
        print("ping time : " + str((round(time.time() *1000) - myAirCManager.pingTime)))



    elif(msg.topic == MQTT_AC_MODE):
        print("@@@@@@@@@@@@@@@@@@ AC MODE")
        print(msg.payload)
        if(msg.payload == MQTT_AC_MODE_OFF):
            print("AC MODE OFF")
            mqttClient.publish(MQTT_GREE_PREFIX + "/power/set", 0)   
        if(msg.payload == MQTT_AC_MODE_HEAT):
            print("AC MODE HEAT")
            mqttClient.publish(MQTT_GREE_PREFIX + "/power/set", 1)   
            mqttClient.publish(MQTT_GREE_PREFIX + "/mode/set", "HEAT")  
        if(msg.payload == MQTT_AC_MODE_COOL):
            print("AC MODE COOL")
            mqttClient.publish(MQTT_GREE_PREFIX + "/power/set", 1)   
            mqttClient.publish(MQTT_GREE_PREFIX + "/mode/set", "COOL")  
        if(msg.payload == MQTT_AC_MODE_FAN):
            print("AC MODE FAN")
            mqttClient.publish(MQTT_GREE_PREFIX + "/power/set", 1)   
            mqttClient.publish(MQTT_GREE_PREFIX + "/mode/set", "FAN")  
	    	    
    else:
        myjson = json.loads(msg.payload)
        current_temperature = myjson['temperature']
        print ("New temperature for " + msg.topic + " : " + str(current_temperature))
        roomList[MQTT_TO_ROOM[msg.topic]].setTemperature(current_temperature)



#=========================================================================#
# Main...                                                                 #
#=========================================================================#  
def main():




    mqttClient.on_connect = on_connect
    mqttClient.on_message = on_message

    mqttClient.connect("localhost")




    aircManagerThread = Thread(target=myAirCManager.aircManagerLoop, args=(mqttClient,))    
    aircManagerThread.start() 

    watchdogThread = Thread(target=myAirCManager.watchdog, args=(mqttClient,))    
    watchdogThread.start() 

    greeStateMonitorThread = Thread(target=myAirCManager.greeStateMonitor, args=(mqttClient,))    
    greeStateMonitorThread.start() 



    # Blocking call that processes network traffic, dispatches callbacks and
    # handles reconnecting.
    # Other loop*() functions are available that give a threaded interface and a
    # manual interface.
    mqttClient.loop_forever()

if __name__ == '__main__':
    main()
