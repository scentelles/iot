import time
from AirCDefines import *
from Room import *

AERO_INIT		= 0
AERO_IDLE 		= 1
AERO_CONFIG_ONGOING 	= 2
AERO_CONFIGURED 	= 3
MQTT_SUFFIX_TARGETTEMP = "targettemp"
MQTT_SUFFIX_AC_STATE = "state"
MQTT_PREFIX = "AC"
AC_STATE_OFF = 1


MQTT_GREE_PREFIX = "AC/GREE"

class AirCManager:

   
    def __init__(self, mqttClient, roomlist):
        self.roomList = roomlist
        self.in_demand = False
        self.aeraulicState = AERO_INIT
        self.ACRunning = False
        self.mqttClient = mqttClient
        self.initDone = False

    def initAfterBoot(self):

#TODO : check ESP is alive

        for r in self.roomList: 
            self.mqttClient.publish(MQTT_PREFIX + "/" + r + "/" + MQTT_SUFFIX_AC_STATE, AC_STATE_OFF)   
            self.mqttClient.publish(MQTT_PREFIX + "/" + r + "/" + MQTT_SUFFIX_TARGETTEMP, 24)  
            self.mqttClient.publish("AC/ESP/SERVO/" + r + "/ANGLE", 0) #can be removed when ESP will send live angle
        self.mqttClient.publish("AC/ESP/SERVO/MASTER2/ANGLE", 0) #can be removed when ESP will send live angle

        self.mqttClient.publish("AC/ESP/SERVO/RESET", 1)
	    
    def isAnyAeroAngleStaged(self):
        result = False
        for r in self.roomList: 
            if(self.roomList[r].aeroChannel.isAngledStaged()):       
                result = True
        return result

    def clearAllAeroStaged(self):
        for r in self.roomList: 
            self.roomList[r].aeroChannel.clearAngledStaged()   

#======================
#Main Loop	
#======================		
    def aircManagerLoop(self, mqttClient):

        time.sleep(1)
        self.initAfterBoot()

        while(self.initDone == False):
            print("Init ongoing")  
            time.sleep(1)
	
        while True:
            print ("aircManager loop\n")
            print ("checking demand\n")

	    #Check and update first each room demand state
            for r in self.roomList:
                self.roomList[r].updateDemand()
		
	    #manage the master channels separately
            for r in self.roomList:
                thisMasterChannel = self.roomList[r].aeroChannel.masterChannel
                if(thisMasterChannel != 0):
                    print("#NB master open :" + str(thisMasterChannel.nbOpen))
                    if(thisMasterChannel.nbOpen != 0):
                        thisMasterChannel.stageOpenChannel()
                    else:
                        thisMasterChannel.stageCloseChannel()		    
	    
            if(self.isACInDemand() == True):
                print("At least one room is in demand")
                if(self.isAnyAeroAngleStaged() == True):
                    self.runAeraulicConfig()
		
                if(self.aeraulicState == AERO_CONFIGURED):
                    if(self.ACRunning == False):
                        self.updateACMastertargetTemp()
                        self.turnACOn()
		
            else:
                print("No demand")
                self.turnACOff()          

            for r in self.roomList:
                self.roomList[r].dumpValues()
            time.sleep(10)    


    def isACInDemand(self):
        demand = False
        for r in self.roomList:
            if (self.roomList[r].isInDemand()):
                print("found room " + self.roomList[r].name + " in demand : " + str(self.roomList[r].getDeltaTemperature()) + "\n")
                demand = True
        self.in_demand = demand
        return self.in_demand

    def runAeraulicConfig(self):
        print("####################### aeroconfigstate : " + str(self.aeraulicState))
        if(self.aeraulicState != AERO_CONFIG_ONGOING):
            self.aeraulicState = AERO_CONFIG_ONGOING
            self.clearAllAeroStaged()
            self.mqttClient.publish("AC/ESP/SERVO/RUN_ALL", 1)
            while(self.aeraulicState == AERO_CONFIG_ONGOING):
                print("Waiting for end of Servo moves")
                time.sleep(1)
			    
        else: 
            print("WARNING : AERO config asked while it was already ongoing")
	      	    
    def getMaxDeltaTemp(self):
       result = 0
       for r in self.roomList:
           thisRoom = self.roomList[r]
           if(thisRoom.isInDemand()):
               if(thisRoom.getDeltaTemperature() > result):
                   result = thisRoom.getDeltaTemperature()     
       return result

    def calculatefanSpeedd(self):
       totalVolumeInDemand = 0
       for r in self.roomList:
           thisRoom = self.roomList[r]
           if(thisRoom.isInDemand()):           
               totalVolumeInDemand += thisRoom.volume
       print("########## : Volume total : " + str(totalVolumeInDemand))
 
       if(totalVolumeInDemand < 26):
           return "low"
       elif(totalVolumeInDemand < 36):
           return "mediumLow"
       elif(totalVolumeInDemand < 51):
           return "medium"
       elif(totalVolumeInDemand < 80):
           return "mediumHigh"	   
       else:
           return "high"	
	   	   
    def turnACOn(self):
        print("AC ON")
        self.ACRunning == True
        self.mqttClient.publish(MQTT_GREE_PREFIX + "/mode/set", "cool")   
        self.mqttClient.publish(MQTT_GREE_PREFIX + "/power/set", 1)   
        self.mqttClient.publish(MQTT_GREE_PREFIX + "/fanspeed/set", self.calculatefanSpeedd())    

    def turnACOff(self):
        print("AC OFF")
        self.ACRunning == False
        self.mqttClient.publish(MQTT_GREE_PREFIX + "/fanspeed/set", "low")
        self.mqttClient.publish(MQTT_GREE_PREFIX + "/power/set", 0)   
	
    def updateACMastertargetTemp(self):
        self.mqttClient.publish(MQTT_GREE_PREFIX + "/temperature/set", self.getMaxDeltaTemp())   

