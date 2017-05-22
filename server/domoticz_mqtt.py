import paho.mqtt.client as mqtt
import ConfigParser
import json
import time


def on_connect(client, userdata, flags, rc):
    print("Connected with result code "+str(rc))
    mqttClient.subscribe("domoticz/out")
    mqttClient.subscribe("ROOM1_SENSOR/temp")
    mqttClient.subscribe("ROOM1_SENSOR/hum")
    mqttClient.subscribe("ECS/temp1")
    mqttClient.subscribe("ECS/temp2")
    mqttClient.subscribe("FLOWER1_WATER/moisture")
    mqttClient.subscribe("POOL/temp")


    mqttClient.subscribe("POOL_PUMP/command")

    
def on_message(client, userdata, msg):
   # print(msg.topic+" "+str(msg.payload)+"\n")
    
    if msg.topic == "ROOM1_SENSOR/temp":
        value = '{ "idx" : 4, "nvalue" : 0, "svalue" : "' + msg.payload + '"}'
        mqttClient.publish("domoticz/in", payload=value)

    if msg.topic == "ROOM1_SENSOR/hum":
        value = '{ "idx" : 41, "nvalue" : ' + str(int(float(msg.payload))) +', "svalue" : "OK"}'
	print "Dreamroom humidity to be sent : " 
	print value
        mqttClient.publish("domoticz/in", payload=value)


    if msg.topic == "ECS/temp1":
        value = '{ "idx" : 1, "nvalue" : 0, "svalue" : "' + msg.payload + '"}'
        mqttClient.publish("domoticz/in", payload=value)
    if msg.topic == "ECS/temp2":
        value = '{ "idx" : 2, "nvalue" : 0, "svalue" : "' + msg.payload + '"}'
        mqttClient.publish("domoticz/in", payload=value)

    if msg.topic == "FLOWER1_WATER/moisture":
        print ("received moisture update")
        value = '{ "idx" : 27, "nvalue" : 0, "svalue" : "' + msg.payload + '"}'
        mqttClient.publish("domoticz/in", payload=value)
        value = '{ "idx" : 28, "nvalue" : 0, "svalue" : "' + msg.payload + '"}'
        mqttClient.publish("domoticz/in", payload=value)

    if msg.topic == "POOL/temp":
        value = '{ "idx" : 42, "nvalue" : 0, "svalue" : "' + msg.payload + '"}'
        mqttClient.publish("domoticz/in", payload=value)


    if msg.topic == "POOL_PUMP/command":
       print ("received Pool pump external update")
       if msg.payload == '2':
           boolVal = "1"
	   print 'External update : switch On '
       else:
           boolVal = "0"
	   print 'External update : switch Off'
	   
       value = '{ "idx" : 38, "nvalue" : ' + boolVal + ', "svalue1" : "0"}'
       mqttClient.publish("domoticz/in", payload=value)

    
    if msg.topic == "domoticz/out":
       # print msg.topic
       # print msg.payload
        data = json.loads(msg.payload)
        index = data["idx"]
        nvalue = data["nvalue"]


        name = data["name"]
        topic = name + "/command"
        if name == "dreamroom/chauffage":
            print "Sending command to MQTT broker on topic :"
	    print topic
            if nvalue == 1:
	        value = '1'
	    else:
	        value = '2'
            mqttClient.publish(topic, payload=value, qos=1, retain=False)
		    
        if name == "ECS Control":
            svalue = data["svalue1"]
	    topic = "ECS/force"
            print "Sending command to MQTT broker on topic :"
	    print topic
            if svalue == "0": #OFF
	        value = '1'
	    elif svalue == "10": #ON
	        value = '2'
	    elif svalue == "20": #AUTO
	        #First force OFF in case the ECS was ON.
	        mqttClient.publish(topic, payload='1', qos=1, retain=False)
	        time.sleep(1)
		value = '0'
            else:
	        print "ERROR : unrecognized svalue in ECS Control"
	        value = '0' 
	    
            mqttClient.publish(topic, payload=value, qos=1, retain=False)

        if name == "Arrosage":
            print "Sending command to MQTT broker on topic :"
	    topic="GARDEN_WATERING/command"
	    print topic
            if nvalue == 1:
	        value = '2'
	    else:
	        value = '1'
            mqttClient.publish(topic, payload=value, qos=1, retain=False)
 
        if name == "Pompe piscine":
            print "Sending command to MQTT broker on topic :"
	    topic="POOL_PUMP/command"
	    print topic
            if nvalue == 1:
	        value = '2'
	    else:
	        value = '1'
            #unscubscribe to avoid infinite loop between in and out topic...
            mqttClient.unsubscribe("POOL_PUMP/command")

            mqttClient.publish(topic, payload=value, qos=1, retain=False)
 
            mqttClient.subscribe("POOL_PUMP/command")
 
        
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
