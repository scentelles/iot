import time

class AeroChannel:
    def __init__(self, mqttClient, masterChannel, name):  
        self.mqttClient = mqttClient
        self.masterChannel = masterChannel
        self.currentAngle = 0
        self.name = name
        self.nbOpen = 0
        self.angleStaged = False

    def stageAngle(self, angle):
        if(self.currentAngle != angle):
            self.currentAngle = angle
            self.mqttClient.publish("AC/ESP/SERVO/" + self.name + "/ANGLE", int(angle))
            self.angleStaged = True
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

    def isAngledStaged(self):
        return self.angleStaged
	
    def clearAngledStaged(self):
        self.angleStaged = False