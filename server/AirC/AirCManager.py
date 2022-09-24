import time
from Room import *
from AirCDefines import *

AERO_INIT		= 0
AERO_IDLE 		= 1
AERO_CONFIG_ONGOING 	= 2
AERO_CONFIGURED 	= 3
MQTT_SUFFIX_TARGETTEMP = "targettemp"
MQTT_SUFFIX_AC_STATE = "state"
MQTT_PREFIX = "AC"
AC_STATE_OFF = 1

SAFETY_ROOM_CHANNEL = "ETAGE"

MQTT_GREE_PREFIX = "AC/GREE"



class AirCManager:

   
    def __init__(self, mqttClient, roomlist):
        self.roomList = roomlist
        self.in_demand = False
        self.aeraulicState = AERO_INIT
        self.ACRunning = False
        self.mqttClient = mqttClient
        self.initDone = False
        self.pingAck = False
        self.FSMState = STATE_INIT
        self.ESP_Connected = False
	
    def initAfterBoot(self):


        self.mqttClient.publish("AC/ERROR", "INITIALIZING CONNECTION")

        for r in self.roomList: 
            roomList[r].aeroChannel.init()
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

#==========================
# watchdog thread
#==========================
    def watchdog(self, mqttClient):
        while(1):
            print("!!!!!!!!!!!!!  PINGING ESP  !!!!!!!!!!!!!")
            self.mqttClient.publish("AC/ESP/PING", 1)
            self.pingAck = False

            time.sleep(3)
            if(self.pingAck == False):
                self.mqttClient.publish("AC/ERROR", "ESP NOT RESPONDING!!!")
                self.ESP_Connected = False
            else:
                self.ESP_Connected = True
		
		
#======================
#Main Loop	
#======================		
    def aircManagerLoop(self, mqttClient):

      while(1):
        print("DEBUG"  + str(self.FSMState) + " CONNECTED : " + str(self.ESP_Connected) )
	
        if(self.FSMState == STATE_INIT):
            print("============== STATE_INIT") 
            if(self.ESP_Connected == True):
                self.turnACOff()
                self.initAfterBoot()
                self.FSMState = STATE_WAIT_ESP_INIT
                self.mqttClient.publish(MQTT_ESP_HOST_INIT_REQUEST, 1)
          
            else:
                self.mqttClient.publish("AC/ERROR", "TRYING TO RECONNECT")


        if(self.FSMState == STATE_WAIT_ESP_INIT):
            print("============== WAIT_ESP_INIT") 
            if(self.ESP_Connected == False):
                 self.FSMState = STATE_INIT
          		
			
        if(self.FSMState == STATE_READY):
            if(self.ESP_Connected == False):
                self.FSMState = STATE_INIT
            else:
                print("============== STATE_READY") 
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
                        self.roomList[SAFETY_ROOM_CHANNEL].aeroChannel.clearSafetyFlag()

                        self.runAeraulicConfig()

                    if(self.aeraulicState == AERO_CONFIGURED):
                        if(self.ACRunning == False):
                            self.updateACMastertargetTemp()
                            self.turnACOn()

                else:
                    print("No demand")
                    self.roomList[SAFETY_ROOM_CHANNEL].aeroChannel.safetyOpen()
                    if(self.ACRunning == True):
                        self.turnACOff()          

                for r in self.roomList:
                    self.roomList[r].dumpValues()


        print("looping\n")
        time.sleep(2)
	
	
	

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
          #  while(self.aeraulicState == AERO_CONFIG_ONGOING): 
          #      print("Waiting for end of Servo moves")
          #      time.sleep(1)
			    
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
        self.ACRunning = True
        self.mqttClient.publish(MQTT_GREE_PREFIX + "/mode/set", "cool")  
        time.sleep(0.5) 
        self.mqttClient.publish(MQTT_GREE_PREFIX + "/power/set", 1)   
        time.sleep(0.5) 
        self.mqttClient.publish(MQTT_GREE_PREFIX + "/fanspeed/set", self.calculatefanSpeedd())    

    def turnACOff(self):
        print("AC OFF")
        self.ACRunning = False
        self.mqttClient.publish(MQTT_GREE_PREFIX + "/fanspeed/set", "low")
        self.mqttClient.publish(MQTT_GREE_PREFIX + "/power/set", 0)   
	
    def updateACMastertargetTemp(self):
        self.mqttClient.publish(MQTT_GREE_PREFIX + "/temperature/set", self.roomList["ETAGE"].temperature + self.getMaxDeltaTemp())   

