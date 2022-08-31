import time
from AirCDefines import *
from Room import *


class AirCManager:

    def __init__(self, roomlist):
        self.roomList = roomlist
        self.in_demand = False
	
    def aircManagerLoop(self, mqttClient):
        while True:
            print ("aircManager loop\n")
            print ("checking demand\n")
            if(self.isACInDemand() == True):
                print("Demand update")
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
