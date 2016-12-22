from time import sleep
from threading import Thread
import paho.mqtt.client as mqtt

MQTT_ADDRESS = "localhost"



# The callback for when the client receives a CONNACK response from the server.
def on_connect(client, userdata, flags, rc):
    print("Connected with result code "+str(rc) + "\n")

def on_message(client, userdata, msg):
    print(msg.topic+" "+str(msg.payload) + "\n")

def ecsMqttMonitor():
    mqttClient = mqtt.Client()
    mqttClient.on_connect = on_connect
    mqttClient.on_message = on_message
    mqttClient.connect(MQTT_ADDRESS)
    mqttClient.subscribe("ECS/temp")
    mqttClient.loop_forever()
   
    
def calendarMonitor():
    while True:
        print "second thread"
        sleep(1)
        
def heatMgr():
    mqttClient = mqtt.Client()
    mqttClient.on_connect = on_connect
    mqttClient.on_message = on_message
    mqttClient.connect(MQTT_ADDRESS)
    mqttClient.loop_start()
    
t = Thread(target=ecsMqttMonitor, args=())
t.start()
t2 = Thread(target=calendarMonitor, args=())
t2.start()
t3 = Thread(target=heatMgr, args=())
t3.start()