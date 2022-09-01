import paho.mqtt.client as mqtt
import time

from threading import Thread

import json

from AirCDefines import *
from Room import *
from AeroChannel import *



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
    else:
        myjson = json.loads(msg.payload)
        current_temperature = myjson['temperature']
        print ("New temperature for " + msg.topic + " : " + str(current_temperature))
        roomList[MQTT_TO_ROOM[msg.topic]].setTemperature(current_temperature)



#=========================================================================#
# Main...                                                                 #
#=========================================================================#  
def main():
    mqttClient = mqtt.Client()
    mqttClient.on_connect = on_connect
    mqttClient.on_message = on_message

    mqttClient.connect("localhost")

    myAirCManager = AirCManager(mqttClient, roomList)
    
    aircManagerThread = Thread(target=myAirCManager.aircManagerLoop, args=(mqttClient,))    
    aircManagerThread.start() 


    masterChannel1 = AeroChannel(mqttClient, 0, "MASTER1") #useless
    masterChannel2 = AeroChannel(mqttClient, 0, "MASTER2")
    masterChannel3 = AeroChannel(mqttClient, 0, "MASTER3") #useless


    roomList[CHAMBRE1] = Room(mqttClient, CHAMBRE1, 25, masterChannel2)
    roomList[CHAMBRE2] = Room(mqttClient, CHAMBRE2, 25, masterChannel2)
    roomList[CHAMBRE3] = Room(mqttClient, CHAMBRE3, 25, masterChannel2)
    roomList[DREAMROOM] = Room(mqttClient, DREAMROOM, 35, masterChannel2)
    roomList[SALON] = Room(mqttClient, SALON, 80, 0)
    roomList[ETAGE] = Room(mqttClient, ETAGE, 35, 0)



    # Blocking call that processes network traffic, dispatches callbacks and
    # handles reconnecting.
    # Other loop*() functions are available that give a threaded interface and a
    # manual interface.
    mqttClient.loop_forever()

if __name__ == '__main__':
    main()
