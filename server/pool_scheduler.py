import paho.mqtt.client as mqtt
from threading import Thread
import time
import datetime

global current_temperature


# The callback for when the client receives a CONNACK response from the server.
def on_connect(client, userdata, flags, rc):
    print("Connected with result code "+str(rc))
    client.subscribe("POOL/temp")


def on_message(client, userdata, msg):
    global current_temperature
    print(msg.topic+" "+str(msg.payload)+"\n")
    if msg.topic == "POOL/temp":
         print "\treceived temperature update "
	 current_temperature = float(str(msg.payload))
	 print current_temperature
	 
def poolPumpScheduler(mqttClient):
    while True:
        print "checking to run pump"
        time.sleep(1)

        now = datetime.datetime.now().strftime('%H:%M')
        print now

def main():
    mqttClient = mqtt.Client()
    mqttClient.on_connect = on_connect
    mqttClient.on_message = on_message

    mqttClient.connect("localhost")

    PoolPumpSchedulerThread = Thread(target=poolPumpScheduler, args=(mqttClient,))
    PoolPumpSchedulerThread.start() 
 

    mqttClient.loop_forever()

if __name__ == '__main__':
    main()
