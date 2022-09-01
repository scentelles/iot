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
        self.stageAngle(0)
        if(self.masterChannel!=0):
            print("Nb open master : " + str(self.masterChannel.nbOpen))
            if(self.masterChannel.nbOpen > 1):
                self.masterChannel.nbOpen -= 1
            else:
                self.masterChannel.stageAngle(0)
                self.masterChannel.nbOpen = 0
	    
    def stageOpenChannel(self):
        print("staging open Channel : " + self.name)
        self.stageAngle(90)

        if(self.masterChannel != 0):
            self.masterChannel.stageAngle(90)
            self.masterChannel.nbOpen += 1
