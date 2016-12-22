from w1thermsensor import W1ThermSensor
from time import sleep
import paho.mqtt.client as mqtt


sensor = W1ThermSensor()

myDelay=60

# The callback for when the client receives a CONNACK response from the server.
def on_connect(client, userdata, flags, rc):
    print("Connected with result code "+str(rc) + "\n")

def on_message(client, userdata, msg):
    print(msg.topic+" "+str(msg.payload) + "\n")
    


client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message


client.connect("localhost")

client.loop_start()

while True:
	temperature_in_celsius = sensor.get_temperature()
#	temperature_in_celsius = 2
	print (temperature_in_celsius)
	print ("\n")
	client.publish("ECS/temp", payload=temperature_in_celsius, qos=0, retain=False)
	
	sleep(int(myDelay))

