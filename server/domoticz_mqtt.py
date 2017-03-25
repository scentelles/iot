import paho.mqtt.client as mqtt
import ConfigParser
import json


def on_connect(client, userdata, flags, rc):
    print("Connected with result code "+str(rc))
    mqttClient.subscribe("domoticz/out")
    mqttClient.subscribe("ROOM1_SENSOR/temp")


    
def on_message(client, userdata, msg):
    print(msg.topic+" "+str(msg.payload)+"\n")
    
    if msg.topic == "ROOM1_SENSOR/temp":
        value = "{ 'idx' : 4, 'nvalue' : 0, 'svalue' : '" + msg.payload +"'}"
        mqttClient.publish("domoticz/in", payload=msg.payload)
    
    if msg.topic == "domoticz/out":
        print msg.topic
   # print msg.payload
        data = json.loads(msg.payload)
        index = data["idx"]
        print index
        nvalue = data["nvalue"]
        print nvalue
        name = data["name"]
        print name
        topic = name + "/command"
        if name == "dreamroom/chauffage":
            print "Sending command to MQTT broker on topic :"
	    print topic
            if nvalue == 1:
	        value = '1'
	    else:
	        value = '2'
            mqttClient.publish(topic, payload=value, qos=1, retain=False)
		    
 
 
        
if __name__ == '__main__':
    config = ConfigParser.ConfigParser()
    config.read('myconf.conf')
    mqttAddress = config.get('MQTT', 'mqttAddress')
    mqttPort = config.get('MQTT', 'mqttPort')

    mqttClient = mqtt.Client()
    mqttClient.on_connect = on_connect
    mqttClient.on_message = on_message
    mqttClient.connect("localhost")

    mqttClient.loop_forever()
