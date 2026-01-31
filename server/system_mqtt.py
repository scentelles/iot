from time import sleep
import paho.mqtt.client as mqtt
import psutil

myDelay=10

# The callback for when the client receives a CONNACK response from the server.
def on_connect(client, userdata, flags, rc):
    print("Connected with result code "+str(rc) + "\n")

def on_message(client, userdata, msg):
    print(msg.topic+" "+str(msg.payload) + "\n")
    


client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message


client.connect("192.168.1.27")

client.loop_start()

while True:
	mem_percent = psutil.virtual_memory().percent
	client.publish("SYSTEM/memory", payload=mem_percent, qos=0, retain=False)
	cpu_percent = psutil.cpu_percent()
	client.publish("SYSTEM/cpu", payload=cpu_percent, qos=0, retain=False)
	
	sleep(int(myDelay))

