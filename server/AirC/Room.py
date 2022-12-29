from AirCDefines import *
from AeroChannel import *

DELTATEMP_THRESHOLD = 0.3


class Room:

   def __init__(self, mqttClient, name, volume, masterChannel):
       self.name = name
       self.volume = volume
       self.temperature = 0
       self.temperature_target = 0
       self.AC_ON = 1
       self.in_demand = False 
       self.aeroChannel = AeroChannel(mqttClient, masterChannel, name)   
          
   def setTemperature(self, value):
       self.temperature = float(value)

   def setTemperatureTarget(self, value):
       self.temperature_target = float(value)

   def setAC_ON(self, value):
       self.AC_ON = int(value)
       
           
   def getDeltaTemperature(self, ACMode):
       result = 0
       if(ACMode == AC_MODE_COOL):
           result = round(self.temperature) - self.temperature_target
       if(ACMode == AC_MODE_HEAT):
           result = round(self.temperature_target) - self.temperature	   
       
       if(result > 0):
           return result
       else:
           return 0  
       
   def updateDemand(self, ACMode): 
       self.in_demand = False

       if(self.AC_ON == 2):
         #if(self.getDeltaTemperature(ACMode) > DELTATEMP_THRESHOLD):
         self.in_demand = True
       else:
         self.in_demand = False
       
       if(self.in_demand == True):
           self.aeroChannel.stageOpenChannel()
       else:
           self.aeroChannel.stageCloseChannel()
	   
   def isInDemand(self):	       
       return self.in_demand 
		          
   def dumpValues(self):
        if(DEBUG == True):
          print("ROOM : " + self.name)
          print("\tAC_ON: \t" + str(self.AC_ON))
          print("\ttemperature : \t" + str(self.temperature))
          print("\ttemperature target : \t" + str(self.temperature_target))
          print("\ttemperature delta : \t" + str(self.getDeltaTemperature()))   

   def getDemandPower(self):
       self.getDeltaTemperature() * self.volume
