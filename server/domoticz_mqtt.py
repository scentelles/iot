import paho.mqtt.client as mqtt
import ConfigParser
import json
import time


current_ecs_control = ""
current_pool_pump = ""
current_garden_watering = ""
current_dreamroom_chauffage = ""
current_pool_pump_mode = ""

def on_connect(client, userdata, flags, rc):
    print("Connected with result code "+str(rc))
    mqttClient.subscribe("domoticz/out")
    mqttClient.subscribe("ROOM1_SENSOR/temp")
    mqttClient.subscribe("ROOM1_SENSOR/hum")
    mqttClient.subscribe("ECS/temp1")
    mqttClient.subscribe("ECS/temp2")
    mqttClient.subscribe("ECS/next_start")

    mqttClient.subscribe("FLOWER1_WATER/moisture")
    mqttClient.subscribe("PISCINE/temp")
    mqttClient.subscribe("PISCINE/ph")

  #  mqttClient.subscribe("dreamroom/chauffage/command")
  #  mqttClient.subscribe("POOL_PUMP/command")
  #  mqttClient.subscribe("GARDEN_WATERING/command")
  #  mqttClient.subscribe("ECS/force")
  #  mqttClient.subscribe("POOL_PUMP/mode")
    mqttClient.subscribe("EDF/power")

    
def on_message(client, userdata, msg):
   # print(msg.topic+" "+str(msg.payload)+"\n")
    
    if msg.topic == "ROOM1_SENSOR/temp":
        value = '{ "idx" : 6, "nvalue" : 0, "svalue" : "' + msg.payload + '"}'
        mqttClient.publish("domoticz/in", payload=value)

    if msg.topic == "ROOM1_SENSOR/hum":
        value = '{ "idx" : 7, "nvalue" : ' + str(int(float(msg.payload))) +', "svalue" : "OK"}'
	print "Dreamroom humidity to be sent : " 
	print value
        mqttClient.publish("domoticz/in", payload=value)


    if msg.topic == "ECS/temp1":
        value = '{ "idx" : 4, "nvalue" : 0, "svalue" : "' + msg.payload + '"}'
        mqttClient.publish("domoticz/in", payload=value)
    if msg.topic == "ECS/temp2":
        value = '{ "idx" : 5, "nvalue" : 0, "svalue" : "' + msg.payload + '"}'
        mqttClient.publish("domoticz/in", payload=value)

    if msg.topic == "ECS/next_start":
        value = '{ "idx" : 17, "nvalue" : 0, "svalue" : "' + msg.payload + '"}'
        mqttClient.publish("domoticz/in", payload=value)


    if msg.topic == "FLOWER1_WATER/moisture":
        print ("received moisture update")
        value = '{ "idx" : 27, "nvalue" : 0, "svalue" : "' + msg.payload + '"}'
        mqttClient.publish("domoticz/in", payload=value)
        value = '{ "idx" : 28, "nvalue" : 0, "svalue" : "' + msg.payload + '"}'
        mqttClient.publish("domoticz/in", payload=value)

    if msg.topic == "PISCINE/temp":
        value = '{ "idx" : 14, "nvalue" : 0, "svalue" : "' + msg.payload + '"}'
        mqttClient.publish("domoticz/in", payload=value)
	
    if msg.topic == "PISCINE/ph":
        value = '{ "idx" : 15, "nvalue" : 0, "svalue" : "' + msg.payload + '"}'
        mqttClient.publish("domoticz/in", payload=value)

    if msg.topic == "EDF/power":
        value = '{ "idx" : 12, "nvalue" : 0, "svalue" : "' + msg.payload + '"}'
        mqttClient.publish("domoticz/in", payload=value)


# COMMANDS (BIDIRECTIONAL)
    if msg.topic == "ECS/force":
       global current_ecs_control

       print ("received ECS force external update")
       
       if msg.payload == '1': 
           svalue1 = "0"  
	   nvalue = "2"
	   print 'External update : Force Off '
       elif msg.payload == '2':
           svalue1 = "10"
	   nvalue = "0"
	   print 'External update : Force On'
       elif msg.payload == '0':
           svalue1 = "20"
	   nvalue = "2"
	   print 'External update : Auto'
       else:
           print "Error: Unknown ECS/force payload\r\t"
	   print msg.payload   	   
       value = '{ "command":"switchlight","idx" : 11, "switchcmd":"Set Level", "level": ' +  svalue1 + '}'
       
       if (current_ecs_control == svalue1):
           print "Skipping to send same value to domoticz"
           return
       else:
           current_ecs_control = svalue1;
           mqttClient.publish("domoticz/in", payload=value)

       
    if msg.topic == "GARDEN_WATERING/command":
       global current_garden_watering
       print ("received garden watering external update")
       if msg.payload == '2':
           boolVal = "1"
	   print 'External update : switch On '
       else:
           boolVal = "0"
	   print 'External update : switch Off'
	   
       value = '{ "idx" : 8, "nvalue" : ' + boolVal + ', "svalue1" : "0"}'
 
       if (current_garden_watering == boolVal):
           print "Skipping to send same value to domoticz"
           return
       else:
           current_garden_watering = boolVal;
           mqttClient.publish("domoticz/in", payload=value)

    if msg.topic == "POOL_PUMP/command":
       global current_pool_pump

       print ("received Pool pump externsal update")
       if msg.payload == '2':
           boolVal = "1"
	   print 'External update : switch On '
       else:
           boolVal = "0"
	   print 'External update : switch Off'
	   
       value = '{ "idx" : 9, "nvalue" : ' + boolVal + ', "svalue1" : "0"}'
       if (current_pool_pump == boolVal):
           print "Skipping to send same value to domoticz"
           return
       else:
           current_pool_pump = boolVal;
           mqttClient.publish("domoticz/in", payload=value)

    if msg.topic == "POOL_PUMP/mode":
       global current_pool_pump_mode

       print ("received Pool pump mode external update")
       if msg.payload == '1': 
           current_pool_pump_mode
	   svalue1 = "10"
	   print 'External update : Manual mode '
       elif msg.payload == '2': 
           current_pool_pump_mode
	   svalue1 = "20"
	   print 'External update : Auto mode '
       else:
	   print 'Unknown external update : falling back on Off '
	   svalue1 = "0"
	   
       value = '{ "command":"switchlight","idx" : 43, "switchcmd":"Set Level", "level": ' +  svalue1 + '}'

       if (current_pool_pump_mode == svalue1):
           print "Skipping to send same value to domoticz"
           return
       else:
           current_pool_pump_mode = svalue1;
           mqttClient.publish("domoticz/in", payload=value)




    if msg.topic == "dreamroom/chauffage/command":
       global current_dreamroom_chauffage
       print ("received dreamroom chauffage external update")
       if msg.payload == '1':
           boolVal = "1"
	   print 'External update : switch On '
       else:
           boolVal = "0"
	   print 'External update : switch Off'
	   
       value = '{ "idx" : 13, "nvalue" : ' + boolVal + ', "svalue1" : "0"}'

       if (current_dreamroom_chauffage == boolVal):
           print "Skipping to send same value to domoticz"
           return
       else:
           current_dreamroom_chauffage = boolVal;
           mqttClient.publish("domoticz/in", payload=value)


    
    if msg.topic == "domoticz/out":
        #print msg.topic
        #print msg.payload
        data = json.loads(msg.payload)
        index = data["idx"]
        nvalue = data["nvalue"]


        name = data["name"]
        topic = name + "/command"
        if name == "Dreamroom chauffage":
	    topic = "dreamroom/chauffage/command"
            print "Sending command to MQTT broker on topic :"
	    print topic
            if nvalue == 1:
	        value = '1'
	    else:
	        value = '2'
            mqttClient.publish(topic, payload=value, qos=1, retain=True)

        if name == "Pool Control":
	    try:
                svalue = data["svalue1"]
	    except KeyError, e:
	        print "Error caught"
		return
		
            topic = "POOL_PUMP/mode"
	    
            print "Sending command to MQTT broker on topic :"
	    print topic
            if svalue == "0": #OFF (should remain unused)
	        value = '0'
	    elif svalue == "10": #MANUAL
	        value = '1'
	    elif svalue == "20": #AUTO
	        value = '2'
            else:
	        print "ERROR : unrecognized svalue in Pool Control"
	        value = '0' 

            mqttClient.publish(topic, payload=value, qos=1, retain=True)

		    
        if name == "ECS Control":
	    
	    try:
                svalue = data["svalue1"]
	    except KeyError, e:
	        print "Error caught"
		return


	    
	    topic = "ECS/force"
            print "Sending command to MQTT broker on topic :"
	    print topic
            if svalue == "0": #OFF
	        value = '1'
	    elif svalue == "10": #ON
	        value = '2'
	    elif svalue == "20": #AUTO
	    
	        #First force OFF in case the ECS was ON.
		mqttClient.unsubscribe(topic)
	        mqttClient.publish(topic, payload='1', qos=1, retain=False)
		
	        time.sleep(1)
		value = '0'
            else:
	        print "ERROR : unrecognized svalue in ECS Control"
	        value = '0' 
	 
	    print "value", value
            mqttClient.publish(topic, payload=value, qos=1, retain=True)
	    mqttClient.subscribe(topic)
	    
        if name == "Arrosage":
            print "Sending command to MQTT broker on topic :"
	    topic="GARDEN_WATERING/command"
	    print topic
            if nvalue == 1:
	        value = '2'
	    else:
	        value = '1'
            #unscubscribe to avoid infinite loop between in and out topic...
            mqttClient.unsubscribe(topic)

            mqttClient.publish(topic, payload=value, qos=1, retain=False)
	    
            mqttClient.subscribe(topic)
 
        if name == "Pompe piscine":
	    print "Manual setting of pool : first go back to manual mode :"
	    #Upon on / off press first disable auto mode
	    #topic = "POOL_PUMP/mode"
	    #value = '1'
            #mqttClient.publish(topic, payload=value, qos=1, retain=False)
            #time.sleep(1)
	    
            print "Sending command to MQTT broker on topic :"
	    topic="PISCINE/command"
	    print topic
            if nvalue == 1:
	        value = '2'
	    else:
	        value = '1'
            #unscubscribe to avoid infinite loop between in and out topic...
            mqttClient.unsubscribe(topic)

            mqttClient.publish(topic, payload=value, qos=1, retain=False)
 
            mqttClient.subscribe(topic)
	    

 
        
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
