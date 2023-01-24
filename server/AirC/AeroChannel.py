import time

class AeroChannel:
    def __init__(self, mqttClient, masterChannel, name):  
        self.mqttClient = mqttClient
        self.masterChannel = masterChannel
        self.name = name
        self.init()


    def init(self):
        self.currentAngle = 0
        self.nbOpen = 0
        self.angleStaged = False
        self.safetyOpened = False
        self.safetyAngle = 0
	
    def stageAngle(self, angle):
        if(self.currentAngle != angle):
            self.currentAngle = angle

            self.angleStaged = True


    def flushStagedAngle(self):
            if(self.angleStaged == True):
                print("CURRENT ANGLE : " + str(self.currentAngle))
                print("SAFETY ANGLE : " + str(self.safetyAngle))		
                self.mqttClient.publish("AC/ESP/SERVO/" + self.name + "/ANGLE", int(self.currentAngle + self.safetyAngle))
                print("MQTT angle sent : " + str(self.currentAngle + self.safetyAngle))
                self.angleStaged = False
	    	    	    	
    def stageCloseChannel(self):
       # print("staging close Channel : " + self.name)
        if(self.currentAngle != 0)  :
            self.stageAngle(0)

            if(self.masterChannel != 0):
                    self.masterChannel.nbOpen -=1
		
    def stageOpenChannel(self, angle):
        print("staging open Channel : " + self.name)
        if(self.safetyOpened == True):
            self.resetSafetyAngle() #regular angle set received, reset the remaining safety state if any

        self.stageAngle(angle)

        if(self.masterChannel != 0):
            self.masterChannel.nbOpen +=1



    def isAngledStaged(self):
        return self.angleStaged
	
    def clearAngledStaged(self):
        self.angleStaged = False

    def setSafetyAngle(self, angle):
        if(angle != self.safetyAngle):
            print("\r\r\t\tOPENING SAFETY CHANNEL on room " + self.name + ", WITH ANGLE" + str(angle))
            self.safetyOpened = True
            self.safetyAngle = angle
            self.angleStaged = True

 #   def setSafetyFlag(self):
 #       self.safetyOpened = True

    def resetSafetyAngle(self):
        if(self.safetyOpened == True):
            self.safetyOpened = False
           # self.stageAngle(0)
            self.safetyAngle = 0
            self.angleStaged = True
	    
    def getCurrentAngle(self):
        return self.currentAngle

    def getSafetyFlag(self):
        return self.safetyOpened
