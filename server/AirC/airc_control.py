import paho.mqtt.client as mqtt
import time

from threading import Thread

import json

from AirCDefines import *
from Room import *




# The callback for when the client receives a CONNACK response from the server.
def on_connect(client, userdata, flags, rc):
    print(("Connected with result code "+str(rc)))

    # Subscribing in on_connect() means that if we lose the connection and
    # reconnect then subscriptions will be renewed.

    client.subscribe(MQTT_ADDRESS_CHAMBRE1)
    client.subscribe(MQTT_ADDRESS_CHAMBRE2)
    client.subscribe(MQTT_ADDRESS_CHAMBRE3)
    client.subscribe(MQTT_ADDRESS_DREAMROOM)
    client.subscribe(MQTT_ADDRESS_SALON)
    client.subscribe(MQTT_ADDRESS_ETAGE)
    #client.subscribe(MQTT_TEST)
    
    
    for r in roomList:
        client.subscribe(MQTT_PREFIX + "/" + roomList[r].name + "/" + MQTT_SUFFIX_TARGETTEMP)   
    for r in roomList:
        client.subscribe(MQTT_PREFIX + "/" + roomList[r].name + "/" + MQTT_SUFFIX_AC_STATE)   
    
    
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

    myAirCManager = AirCManager(roomList)
    
    aircManagerThread = Thread(target=myAirCManager.aircManagerLoop, args=(mqttClient,))    
    aircManagerThread.start() 





    # Blocking call that processes network traffic, dispatches callbacks and
    # handles reconnecting.
    # Other loop*() functions are available that give a threaded interface and a
    # manual interface.
    mqttClient.loop_forever()

if __name__ == '__main__':
    main()
