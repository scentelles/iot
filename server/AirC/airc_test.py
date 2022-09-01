import paho.mqtt.client as mqtt
import json


from AirCDefines import *
import time


def setTemperature(client, address, value):

    test_dict = {'temperature': value}

    test_json = json.dumps(test_dict)
    client.publish(address, payload=test_json, qos=0, retain=True)


# The callback for when the client receives a CONNACK response from the server.
def on_connect(client, userdata, flags, rc):
    print(("Connected with result code "+str(rc)))

mqttClient = mqtt.Client()
mqttClient.on_connect = on_connect

mqttClient.connect("localhost")

time.sleep(1)

setTemperature(mqttClient, MQTT_ADDRESS[CHAMBRE1], 25)
setTemperature(mqttClient, MQTT_ADDRESS[CHAMBRE2], 26)
setTemperature(mqttClient, MQTT_ADDRESS[CHAMBRE3], 27)
setTemperature(mqttClient, MQTT_ADDRESS[DREAMROOM], 28)
setTemperature(mqttClient, MQTT_ADDRESS[ETAGE], 28)
setTemperature(mqttClient, MQTT_ADDRESS[SALON], 28)

time.sleep(1)



for r in roomList:
    mqttClient.publish(MQTT_PREFIX + "/" + roomList[r].name + "/" + MQTT_SUFFIX_TARGETTEMP, 22)   
    mqttClient.publish(MQTT_PREFIX + "/" + roomList[r].name + "/" + MQTT_SUFFIX_AC_STATE, AC_STATE_ON)   
    

time.sleep(10)
mqttClient.publish(MQTT_PREFIX + "/" + "DREAMROOM" + "/" + MQTT_SUFFIX_AC_STATE, AC_STATE_OFF)   



print ("Message sent\n")
