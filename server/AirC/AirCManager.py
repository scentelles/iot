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

class AirCManager:

   
    def __init__(self, mqttClient, roomlist):
        self.roomList = roomlist
        self.in_demand = False
        self.aeraulicState = AERO_INIT
        self.ACRunning = False
        self.mqttClient = mqttClient
        self.initDone = False

    def initAfterBoot(self):
        for r in self.roomList: 
            self.mqttClient.publish(MQTT_PREFIX + "/" + self.roomList[r].name + "/" + MQTT_SUFFIX_AC_STATE, AC_STATE_OFF)   
            self.mqttClient.publish(MQTT_PREFIX + "/" + self.roomList[r].name + "/" + MQTT_SUFFIX_TARGETTEMP, 24)  
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
			
    def aircManagerLoop(self, mqttClient):


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
                        self.turnACOn()
		
            else:
                print("No demand")
          

            for r in self.roomList:
                self.roomList[r].dumpValues()
            time.sleep(10)    

    def isACInDemand(self):
        demand = False
        for r in self.roomList:
            if (self.roomList[r].isInDemand()):
                print("found room " + self.roomList[r].name + " in demand : " + str(self.roomList[r].getDeltaTemperature()) + "\n")
                self.in_demand = True
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
	      	    
    def turnACOn(self):
        print("AC ON")
        self.ACRunning == True
