import time
from AirCDefines import *
from Room import *


AERO_IDLE 		= 0
AERO_CONFIG_ONGOING 	= 1
AERO_CONFIGURED 	= 2

class AirCManager:

   
    def __init__(self, mqttClient, roomlist):
        self.roomList = roomlist
        self.in_demand = False
        self.aeraulicState = AERO_IDLE
        self.ACRunning = False
        self.mqttClient = mqttClient
	
    def aircManagerLoop(self, mqttClient):
        while True:
            print ("aircManager loop\n")
            print ("checking demand\n")

	    #Check and update first each room demand state
            for r in self.roomList:
                self.roomList[r].updateDemand()
	    
            if(self.isACInDemand() == True):
                print("At least one room is in demand")
                self.runAeraulicConfig()
		
                if(self.aeraulicState == AERO_CONFIGURED):
                    if(self.ACRunning == False):
                        turnACOn()
		
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
        
	
        if(self.aeraulicState != AERO_CONFIG_ONGOING):
            self.aeraulicState == AERO_CONFIG_ONGOING
            self.mqttClient.publish("AC/SERVORUN/", 1)
        else: 
            print("WARNING : AERO config asked while it was already ongoing")
	      	    
    def turnACOn(self):
        print("AC ON")
