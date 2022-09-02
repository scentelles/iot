import time

class AeroChannel:
    def __init__(self, mqttClient, masterChannel, name):  
        self.mqttClient = mqttClient
        self.masterChannel = masterChannel
        self.currentAngle = 0
        self.name = name
        self.nbOpen = 0

    def stageAngle(self, angle):
        if(self.currentAngle != angle):
            self.currentAngle = angle
            self.mqttClient.publish("AC/SERVO/" + self.name + "/angle", angle)
	    
            print("MQTT angle sent")
	
    def stageCloseChannel(self):
        print("staging close Channel : " + self.name)
        if(self.currentAngle != 0):
            self.stageAngle(0)

            if(self.masterChannel != 0):
                    self.masterChannel.nbOpen -=1
		
    def stageOpenChannel(self):
        print("staging open Channel : " + self.name)
        if(self.currentAngle == 0):
            self.stageAngle(90)

            if(self.masterChannel != 0):
                    self.masterChannel.nbOpen +=1
