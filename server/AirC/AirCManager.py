import time
from Room import *
from AirCDefines import *
import time


AERO_INIT		= 0
AERO_IDLE 		= 1
AERO_CONFIG_ONGOING 	= 2
AERO_CONFIGURED 	= 3
MQTT_SUFFIX_TARGETTEMP = "targettemp"
MQTT_SUFFIX_AC_STATE = "state"
MQTT_PREFIX = "AC"
#AC_STATE_OFF = 1





class AirCManager:

   
    def __init__(self, mqttClient, roomlist):
        self.roomList = roomlist
        self.in_demand = False
        self.aeraulicState = AERO_INIT
        self.ACRunning = False
        self.lastACSwitchOnTime = 0
        self.lastACSwitchOffTime = 0
        self.mqttClient = mqttClient
        self.initDone = False
        self.pingAck = False
        self.FSMState = STATE_INIT
        self.ESP_Connected = False
        self.currentACTempTarget = 0
        self.currentACMode = AC_MODE_OFF
        self.currentFanSpeed = 1
        self.currentGreeAmbiantTemp = 0
        self.currentTurboForced = False
        self.sumOfAngles =0
        self.safetyAngle = 0
        self.masterAlreadyForced = False
        self.HAStarted = False


    def initAfterBoot(self):


        self.mqttClient.publish("AC/ERROR", "INITIALIZING CONNECTION")
	
        self.mqttClient.publish(MQTT_AC_TURBO_FORCED, 1)



        for r in self.roomList: 
            roomList[r].aeroChannel.init()
            self.mqttClient.publish(MQTT_PREFIX + "/" + r + "/" + MQTT_SUFFIX_AC_STATE, AC_STATE_OFF)   
            self.mqttClient.publish(MQTT_PREFIX + "/" + r + "/" + MQTT_SUFFIX_TARGETTEMP, DEFAULT_TARGET_TEMP)  
            self.mqttClient.publish("AC/ESP/SERVO/" + r + "/ANGLE", 0) 
        self.mqttClient.publish("AC/ESP/SERVO/MASTER2/ANGLE", 0) 

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
# greeStateMonitor thread
#==========================
    def greeStateMonitor(self, mqttClient):
        while(1):
            if(self.FSMState != STATE_WAIT_ESP_INIT):
              print("trigger get core status")
              self.mqttClient.publish("AC/GREE/corestatus/get", 1)

              time.sleep(5)

              self.mqttClient.publish("AC/GREE/secondarystatus/get", 1)

              time.sleep(5)                

#==========================
# watchdog thread
#==========================
    def watchdog(self, mqttClient):
        nbConnectionLosss = 0
        while(1):
            if(self.FSMState == STATE_WAIT_ESP_INIT):   #timout of ESP init
                time.sleep(300)
                if(self.FSMState == STATE_WAIT_ESP_INIT):
                    self.FSMState =  STATE_INIT #force reset state, network loss has made the ESP unreachable
		
            else:     #exlude ping while servos are under init
                print("!!!!!!!!!!!!!  PINGING ESP  !!!!!!!!!!!!!")
                self.pingTime = round(time.time() * 1000)
                self.mqttClient.publish("AC/ESP/PING", 1)

                self.pingAck = False
                time.sleep(10)

			    
                if(self.pingAck == False):
                    if(self.ESP_Connected != False):
                      nbConnectionLosss += 1
                      if(nbConnectionLosss == 5):
                        self.ESP_Connected = False
                        self.mqttClient.publish("AC/ERROR", "ESP NOT RESPONDING!!!")
                        nbConnectionLosss = 0
                else:
                    self.ESP_Connected = True
                    nbConnectionLosss = 0
		
		
#======================
#Main Loop	
#======================		
    def aircManagerLoop(self, mqttClient):
      #wait for HA to start      
      while(self.HAStarted == False):
        print("waiting for HA to start\n")
        time.sleep(2) 
	      
      while(1):
        print("DEBUG"  + str(self.FSMState) + " CONNECTED : " + str(self.ESP_Connected) )
	
        if(self.FSMState == STATE_INIT):
            print("============== STATE_INIT") 
            if(self.ESP_Connected == True):
                #self.turnACOff()
                self.initAfterBoot()
                self.FSMState = STATE_WAIT_ESP_INIT
                self.mqttClient.publish(MQTT_ESP_HOST_INIT_REQUEST, 1)
                self.mqttClient.publish("AC/ERROR", "WAITING_ESP_SERVO_INIT")
          
            else:
                self.mqttClient.publish("AC/ERROR", "TRYING TO RECONNECT")


        if(self.FSMState == STATE_WAIT_ESP_INIT):
            print("============== WAIT_ESP_INIT") 
            if(self.ESP_Connected == False):
                 self.FSMState = STATE_INIT
            	
            #self.FSMState = STATE_READY #remove	
	     		
        if(self.FSMState == STATE_READY):
            if(self.ESP_Connected == False):
                self.FSMState = STATE_INIT
            else:
                print("============== STATE_READY") 
                print ("checking demand\n")

                if(self.masterAlreadyForced == False):
                    self.mqttClient.publish("AC/ESP/SERVO/MASTER2/ANGLE", 90) #always open master at init
                    self.masterAlreadyForced = True

                #Check and update first each room demand state
                for r in self.roomList:
                    self.roomList[r].updateDemand(self.currentACMode)

	    
                #manage the master channels separately
            #    for r in self.roomList:
            #        thisMasterChannel = self.roomList[r].aeroChannel.masterChannel
            #        if(thisMasterChannel != 0):
                        #print("#NB master open :" + str(thisMasterChannel.nbOpen))
             #           if(thisMasterChannel.nbOpen != 0):
             #               thisMasterChannel.stageOpenChannel(MAX_OPEN_ANGLE)
            #            else:
                           # thisMasterChannel.stageCloseChannel()
             #               thisMasterChannel.stageOpenChannel(MAX_OPEN_ANGLE)	#finally we want it always opened!	    

                if(self.isACInDemand() == True):
                    print("At least one room is in demand")
                    if(self.isAnyAeroAngleStaged() == True):
                        self.roomList[SAFETY_ROOM_CHANNEL].aeroChannel.resetSafetyAngle()

                    if(self.ACRunning == False):
                        self.turnACOn()

                    self.updateACMastertargetTemp()

                    #check if a partial safety opening must be trigered
                    self.sumOfAngles = 0
                    for r in self.roomList:
                        if self.roomList[r].isInDemand():
                            self.sumOfAngles += self.roomList[r].aeroChannel.getCurrentAngle()
                    tempSafetyAngle = MAX_OPEN_ANGLE - self.sumOfAngles
                    if(tempSafetyAngle > 0):
                        self.safetyAngle = min(tempSafetyAngle, MAX_OPEN_ANGLE)
                        self.roomList[SAFETY_ROOM_CHANNEL].aeroChannel.setSafetyAngle(self.safetyAngle)
                    else:
                        self.roomList[SAFETY_ROOM_CHANNEL].aeroChannel.resetSafetyAngle()
                else:
                    print("\tNo demand")
                    self.roomList[SAFETY_ROOM_CHANNEL].aeroChannel.setSafetyAngle(MAX_OPEN_ANGLE)
                    if(self.ACRunning == True):
                        self.turnACOff()          





                for r in self.roomList:
                    self.roomList[r].dumpValues()
                    self.roomList[r].aeroChannel.flushStagedAngle()

       
        time.sleep(2)
	
	
	

    def isACInDemand(self):
        demand = False
        for r in self.roomList:
            if (self.roomList[r].isInDemand()):
                print("found room " + self.roomList[r].name + " in demand : " + str(self.roomList[r].getDeltaTemperature(self.currentACMode)) + "\n")
                demand = True
        self.in_demand = demand
        return self.in_demand

	      	    
    def getMaxDeltaTemp(self):
       result = 0.0
       for r in self.roomList:
           thisRoom = self.roomList[r]
           if(thisRoom.isInDemand()):
               if(thisRoom.getDeltaTemperature(self.currentACMode) > result):
                   result = thisRoom.getDeltaTemperature(self.currentACMode)     
       print("result - delta temperature : " + str(result))
       return result

    def calculatefanSpeed(self):
       if (self.currentTurboForced == True):
           return GREE_FANSPEED_TURBO
           print("\t\tFORCING TURBO!!!!")

       else:
         totalVolumeInDemand = 0
         for r in self.roomList:
           thisRoom = self.roomList[r]
           if(thisRoom.isInDemand()):           
               totalVolumeInDemand += thisRoom.volume
         print("########## : Volume total : " + str(totalVolumeInDemand))
 
         if(totalVolumeInDemand < 26):
             return GREE_FANSPEED_LOW
         elif(totalVolumeInDemand < 36):
             return GREE_FANSPEED_MEDIUMLOW
         elif(totalVolumeInDemand < 51):
             return GREE_FANSPEED_MEDIUM
         elif(totalVolumeInDemand < 80):
             return GREE_FANSPEED_MEDIUMHIGH
         else:
             if(self.getMaxDeltaTemp() < 0.5):
                 return GREE_FANSPEED_MEDIUMHIGH
             elif(self.getMaxDeltaTemp() < 0.8):
                 return GREE_FANSPEED_HIGH
             else:
                 return GREE_FANSPEED_TURBO
		 

	   	   
    def turnACOn(self):
        print("AC ON request")
        if(self.currentACMode != AC_MODE_OFF):

            tempTime = time.time()

            if(tempTime - self.lastACSwitchOffTime > SWITCH_ON_TIMER_LIMIT):
                print("############################   AC ON   ##########################")
                self.lastACSwitchOnTime = tempTime
                self.ACRunning = True
                self.mqttClient.publish(MQTT_GREE_PREFIX + "/fanspeed/set", self.calculatefanSpeed())    
                self.mqttClient.publish(MQTT_GREE_PREFIX + "/power/set", 1)   
            else:
                print("Skipping AC ON to preserve compressor - too early power request")
                print(str(tempTime - self.lastACSwitchOffTime))
        else:
            print("Warning - AC mode seems inconsistent")

 
    def turnACOff(self):
        tempTime = time.time()

        print("AC OFF request")
        if(tempTime - self.lastACSwitchOnTime > SWITCH_OFF_TIMER_LIMIT):
            print("############################   AC OFF   ##########################")
            self.lastACSwitchOffTime = tempTime
            self.ACRunning = False
            self.mqttClient.publish(MQTT_GREE_PREFIX + "/fanspeed/set", 1)
            self.mqttClient.publish(MQTT_GREE_PREFIX + "/power/set", 0)   
        else:
            print("Skipping AC OFF to preserve compressor - too early power request")
            print(str(tempTime - self.lastACSwitchOnTime))

	
    def updateACMastertargetTemp(self):
        print("############################  SET TEMP  ##########################")
        print("Current ambiant temp : " + str(self.currentGreeAmbiantTemp))
        if(self.currentACMode == AC_MODE_COOL):
          newACTempTarget = round(self.currentGreeAmbiantTemp - self.getMaxDeltaTemp())
          if(self.getMaxDeltaTemp() == 0): #Take into account target has been reached.
            newACTempTarget = newACTempTarget + 1 
        if(self.currentACMode == AC_MODE_HEAT):
          newACTempTarget = round(self.currentGreeAmbiantTemp + self.getMaxDeltaTemp())	  
          if(self.getMaxDeltaTemp() == 0):  #Take into account target has been reached.
            newACTempTarget = newACTempTarget - 1
	    	  
        if(self.currentACTempTarget != newACTempTarget):
           self.currentACTempTarget = newACTempTarget
           self.mqttClient.publish(MQTT_GREE_PREFIX + "/temperature/set", newACTempTarget)   
        time.sleep(0.2) 
	  
        newFanSpeed = self.calculatefanSpeed()
        if(self.currentFanSpeed != newFanSpeed):
             self.currentFanSpeed = newFanSpeed
             self.mqttClient.publish(MQTT_GREE_PREFIX + "/fanspeed/set", newFanSpeed)    
