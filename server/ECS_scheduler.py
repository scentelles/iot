from __future__ import print_function
import httplib2
import os

from apiclient import discovery
from oauth2client import client
from oauth2client import tools
from oauth2client.file import Storage

import datetime
import rfc3339      # for date object -> date string
import iso8601      # for date string -> date object

import ConfigParser
import time
import pytz

import smtplib
from email.MIMEMultipart import MIMEMultipart
from email.MIMEText import MIMEText

from enum import Enum

import paho.mqtt.client as mqtt
from threading import Thread
from Queue import Queue



heatMgrQueue = 0

#TODO unused
class EcsCommand(Enum):
    OFF         = 1
    ON          = 2
   
class EcsForceCommand(Enum):
    DISABLED    = 0
    OFF         = 1
    ON          = 2   

lastTemperatureUpdate = datetime.datetime.now()
currentTemperatureUpdate = datetime.datetime.now()
global configWarningSender, configWarningRecipient, configSmtpLogin, configSmtpPassword
global ecsState, ecsRemoteState, ecsStateForced, ecsTemperature, ecsHeatTarget      
global calendarId

#defines
ECS_HEAT_PROFILE_LOW         = "LOW"
ECS_HEAT_PROFILE_MEDIUM      = "MEDIUM"
ECS_HEAT_PROFILE_HIGH        = "HIGH"

ECS_STATE_OFF = "OFF"
ECS_STATE_ON = "ON"

ECS_FORCE_DISABLED = "FORCE_DISABLED"
ECS_FORCE_OFF = "FORCE_OFF"
ECS_FORCE_ON  = "FORCE_ON"


OVERHEAT_TEMPERATURE = 61
UNDERHEAT_TEMPERATURE = 16

#Globals
nextEventfromCalendar = None


try:
    import argparse
    flags = argparse.ArgumentParser(parents=[tools.argparser]).parse_args()
except ImportError:
    flags = None

# If modifying these scopes, delete your previously saved credentials
# at ~/.credentials/calendar-python-quickstart.json
SCOPES = 'https://www.googleapis.com/auth/calendar.readonly'
CLIENT_SECRET_FILE = 'client_secret.json'
APPLICATION_NAME = 'Client ECS'


HEAT_MANAGER_PERIOD         = 1
DELAY_BETWEEN_SCHED_CHECK   = 1
DELAY_BETWEEN_AGENDA_CHECK  = 10000  # in multiples of SCHED check
NB_EVENTS_TO_GET_FROM_CALENDAR  = 1
DELAY_BETWEEN_TEMPERATURE_UPDATE = 60

class heatMgrMessage:
        def __init__(self, type, value, heatProfile="MEDIUM"):
            self.type  = type
            self.value = value
            self.heatProfile = heatProfile

class ThermoEvent:
        def __init__(self, title, value, start, end):
                self.title = title
                self.value = value
                self.start = start
                self.end = end

#=========================================================================#                
#                      callback definition for MQTT                       #          
#=========================================================================#

# The callback for when the client receives a CONNACK response from the server.
def on_connect(client, userdata, flags, rc):
    print("Connected with result code "+str(rc) + "\n")

def on_message(client, userdata, msg):
    tmpQueue = userdata
    tmpValue = ""
    print ("Callback tmpQueue : ") 
    print (tmpQueue)
    print(msg.topic+" "+str(msg.payload) + "\n")
    if msg.topic == "ECS/force":
        print ("Callback sending force command to heatManager")
        if(msg.payload == '0'):
            tmpValue = ECS_FORCE_DISABLED
        elif(msg.payload == '1'):
            tmpValue = ECS_FORCE_OFF
        elif(msg.payload == '2'):
            tmpValue = ECS_FORCE_ON
        else:
            print ("Callback Error : Unknown Force command :", msg.payload)
        tempMsg = heatMgrMessage('ECS_FORCE', tmpValue)
        tmpQueue.put(tempMsg)
    if msg.topic == "ECS/temp2":
        print("Callback sending temperature to heatManager")
        tempMsg = heatMgrMessage('ECS_TEMPERATURE', msg.payload)
        tmpQueue.put(tempMsg)
    if msg.topic == "ECS/calendar_update":
    	if(msg.payload == '1'):
            getNextEventFromCalendar()


#=========================================================================#   
#           helper functions for timezone and format conversion           #
#=========================================================================#
def get_date_object(date_string):
  return iso8601.parse_date(date_string)

def get_date_string(date_object):
  return rfc3339.rfc3339(date_object)  

   
		
#=========================================================================#
# Functions for Google calendar data retrieval                            #
# Coming from Google API example                                          #
# https://developers.google.com/google-apps/calendar/quickstart/python    #
#=========================================================================#

def get_credentials():
    """Gets valid user credentials from storage.

    If nothing has been stored, or if the stored credentials are invalid,
    the OAuth2 flow is completed to obtain the new credentials.

    Returns:
        Credentials, the obtained credential.
    """
    home_dir = os.path.expanduser('~')
    credential_dir = os.path.join(home_dir, '.credentials')
    if not os.path.exists(credential_dir):
        os.makedirs(credential_dir)
    credential_path = os.path.join(credential_dir,
                                   'calendar-python-quickstart.json')

    store = Storage(credential_path)
    credentials = store.get()
    if not credentials or credentials.invalid:
        flow = client.flow_from_clientsecrets(CLIENT_SECRET_FILE, SCOPES)
        flow.user_agent = APPLICATION_NAME
        if flags:
            credentials = tools.run_flow(flow, store, flags)
        else: # Needed only for compatibility with Python 2.6
            credentials = tools.run(flow, store)
        print('Storing credentials to ' + credential_path)
    return credentials

def turnEcsOn(): print ("Turning ECS ON :", time.time())

#TODO retrieve weekly calendar once for all
def getEventsFromCalendar(calendarId):
    credentials = get_credentials()
    http = credentials.authorize(httplib2.Http())
    service = discovery.build('calendar', 'v3', http=http)

    now = datetime.datetime.utcnow().isoformat() + 'Z' # 'Z' indicates UTC time
	
    mynow  = datetime.datetime.utcnow()
    myweek = datetime.timedelta(days=7)

    start_date = mynow.isoformat() + 'Z'
    end_date = (mynow + myweek).isoformat() + 'Z'

    print('Getting the upcoming events')
    eventsResult = service.events().list(
        calendarId=calendarId, timeMin=start_date, timeMax=end_date, maxResults=NB_EVENTS_TO_GET_FROM_CALENDAR, singleEvents=True, 
        orderBy='startTime').execute()
    events = eventsResult.get('items', [])

    return events

   
   
   
#=========================================================================#   
#           Heat profile to temperature conversion                        #
#=========================================================================#
        
def getTargetTemperature(profile):
    if (profile == ECS_HEAT_PROFILE_HIGH):
        return 60
    elif (profile == ECS_HEAT_PROFILE_MEDIUM):
        return 55
    elif (profile == ECS_HEAT_PROFILE_LOW):
        return 48
    else:
        print("Unknown heatprofile. Defaulting to 50", profile)
        return 53
        
        
def getStatusString():
    global ecsState, ecsRemoteState, ecsStateForced, ecsTemperature, ecsHeatTarget
    result  = "\n\n====================="
    result += "\n\tECS state :" + ecsState
    result += "\n\tECS remote state :" + ecsRemoteState
    result += "\n\tECS force state :" + str(ecsStateForced)
    result += "\n\tECS temperature :" + str(ecsTemperature)
    result += "\n\tTarget temperature : " + str(ecsHeatTarget)
    result += "\n====================="
    return result


def warnMessage(msg):
    Mimemsg = MIMEMultipart()
    global configWarningSender, configWarningRecipient, configSmtpLogin, configSmtpPassword
    
    Mimemsg['From'] = configWarningSender
    Mimemsg['To'] = configWarningRecipient
    Mimemsg['Subject'] = 'ECS WARNING' 
    msg += getStatusString()
    Mimemsg.attach(MIMEText(msg))
    server = smtplib.SMTP('smtp.gmail.com', 587)
    server.ehlo()
    server.starttls()
    server.login(configSmtpLogin, configSmtpPassword)
    server.sendmail(configWarningSender, configWarningRecipient, Mimemsg.as_string())
    server.quit()

def checkTemperatureValidity(temperature):  
    if(temperature < UNDERHEAT_TEMPERATURE):
        warnMessage("Warning, the temperature of the ECS is getting low. Consider forcing ON")
    if(temperature > OVERHEAT_TEMPERATURE):
        warnMessage("Warning, the temperature of the ECS is getting too high. Consider forcing OFF")


    
#=========================================================================#
# Heat manager thread body                                                #
# processes messages from Queue :                                         # 
#       - ECS state change requests (from Ecs Scheduler thread)           #
#       - ECS force state (from MQTTLoop thread)                          #
#       - temperature(s)  (from MQTTLoop thread)                          #
#=========================================================================#
def heatManager(msqQueue, mqttClient):
    global ecsState, ecsRemoteState, ecsStateForced, ecsTemperature, ecsHeatTarget
    global lastTemperatureUpdate, currentTemperatureUpdate
    ecsState = ECS_STATE_OFF 
    ecsRemoteState = ECS_STATE_OFF 
    ecsStateForced = False
    ecsTemperature = 0
    ecsHeatTarget = 0

    
    while True:
        print ("HeatManager waiting for message")
        msg = msqQueue.get()
        msgType = msg.type
        msgValue = msg.value
        msgHeatProfile = msg.heatProfile

        print("HeatManager waking up. message received")
        print("\tmsgtype : ", msgType)
        print ("\tmsgvalue :", msgValue)
        print ("\tmsgheatprofile :", msgHeatProfile)
        print(getStatusString())
       
        
        
        
            
        #process messages
        if(msgType == "ECS_STATE_CHANGE"):
            if (ecsStateForced is False):
                if (msgValue == ECS_STATE_OFF):
                    print("Heat Manager : turning ECS OFF")
                    ecsState = ECS_STATE_OFF 
                    mqttClient.publish("ECS/state", payload='1', qos=1, retain=False)
		    mqttClient.publish("ECS/target", payload='0', qos=1, retain=False)

                elif (msgValue == ECS_STATE_ON):
                    ecsState = ECS_STATE_ON
                    ecsHeatTarget = getTargetTemperature(msgHeatProfile)
                    
                    #Check if temperature is recent enough to be valid, else warn user
                    currentTime = datetime.datetime.now()
                    deltaTime = currentTime - currentTemperatureUpdate
                    deltaTimeInSeconds = deltaTime.total_seconds()
                    if(deltaTimeInSeconds > DELAY_BETWEEN_TEMPERATURE_UPDATE *10):
                        message = "Warning : Switching ECS while temperature info may not be valid : \nsensor update exceeded 10 times maximum delta time: " + str(deltaTimeInSeconds) + "seconds"
                        warnMessage(message)  
                        print(message)
                    
                    if(ecsTemperature < ecsHeatTarget):
                        print("Heat Manager : turning ECS ON")
                        mqttClient.publish("ECS/state", payload='2', qos=1, retain=False)
			mqttClient.publish("ECS/target", payload=ecsHeatTarget, qos=1, retain=False)
			
                        ecsRemoteState  = ECS_STATE_ON
                    else:
                        print("Heat Manager : No ECS ON despite calendar, due to target temperature already reached")
                else: 
                    print("Heat Manager : Error : unknown EcsState %s in received message" % msgValue)

        elif(msgType == "ECS_TEMPERATURE"):
            ecsTemperature = float(msgValue)

            lastTemperatureUpdate = currentTemperatureUpdate
            currentTemperatureUpdate = datetime.datetime.now()
            deltaTime = currentTemperatureUpdate - lastTemperatureUpdate
            deltaTimeInSeconds = deltaTime.total_seconds()
            
            if(deltaTimeInSeconds > DELAY_BETWEEN_TEMPERATURE_UPDATE *2):
                message = "Warning : Temperature update from sensor exceeded twice maximum delta time: " + str(deltaTimeInSeconds) + "seconds"
                print(message)
                warnMessage(message)
            
            
            
            print ("updating temperature : ", msgValue)
            checkTemperatureValidity(ecsTemperature)
            #Check against temperature target when ECS is ON and not in forced mode
            if ((ecsState == ECS_STATE_ON) and (ecsStateForced is False)):
                if (ecsTemperature > ecsHeatTarget):
                    print("Heat Manager : Switching ECS OFF despite calendar, due to target temperature reached")
                    ecsState = ECS_STATE_OFF
                    mqttClient.publish("ECS/state", payload='1', qos=1, retain=True)
		    mqttClient.publish("ECS/target", payload='0', qos=1, retain=False)

                    ecsRemoteState  = ECS_STATE_OFF
 
        elif(msgType == "ECS_FORCE"):   
            print ("HeatMgr : ecsState",  ecsState)
            if (msgValue == ECS_FORCE_OFF):
                print("Heat Manager : Forcing ECS OFF") 
                ecsStateForced = True
                if (ecsState == ECS_STATE_ON):
                    print("\tHeat Manager : Switching ECS OFF") 
                    mqttClient.publish("ECS/state", payload='1', qos=1, retain=False)
		    mqttClient.publish("ECS/target", payload='0', qos=1, retain=False)

                    ecsState = ECS_STATE_OFF
                    ecsRemoteState  = ECS_STATE_OFF
            elif (msgValue == ECS_FORCE_ON):
                print("Heat Manager : Forcing ECS ON") 
                ecsStateForced = True
                if (ecsState == ECS_STATE_OFF):
                    print("\tHeat Manager : Switching ECS ON") 
                    mqttClient.publish("ECS/state", payload='2', qos=1, retain=False)
		    mqttClient.publish("ECS/target", payload='100', qos=1, retain=False)

                    ecsState = ECS_STATE_ON
                    ecsRemoteState  = ECS_STATE_ON
            elif (msgValue == ECS_FORCE_DISABLED):
                print("Heat Manager : Disabling Forcing ECS") 
                ecsStateForced = False
            else:  
                print("Heat Manager : Unknown message value %s " % msgValue)
        
        else:
            print("Heat Manager : Unknown message type %s " % msgType)




def getNextEventFromCalendar():
    global nextEventfromCalendar 
    global calendarId
    events = getEventsFromCalendar(calendarId)
    nextEventfromCalendar = None
    if not events:
        print('No upcoming events found.')
    else:
        myevent = events[0]
        title = myevent['summary']
        content = ""
        start = get_date_object(myevent['start'].get('dateTime'))
        end   = get_date_object(myevent['end'].get('dateTime'))
        nextEventfromCalendar = ThermoEvent(title, content, start, end)
 
#=========================================================================#
# Calendar monitor thread body                                            #
# Periodically checks for events defined in cloud                         # 
#=========================================================================#
def calendarMonitor(calendarId):
    while True:
        getNextEventFromCalendar()
        time.sleep(DELAY_BETWEEN_AGENDA_CHECK)


        
#=========================================================================#
# ECS State scheduler                                                     #
# Sends commands to heatManager based on periodical checks of             #
# next event defined by Calendar monitor                                  # 
#=========================================================================#
def ecsStateScheduler(heatMgrQueue, mqttClient):
    logNoEventDisplayed     = False
    ecsState = ECS_STATE_OFF
    nextStartTime = ""
    while True:
        if(nextEventfromCalendar):
            parisTz = pytz.timezone('Europe/Paris')
            now = parisTz.localize(datetime.datetime.now())
            #print ("START : ", nextEventfromCalendar.start)
            #print ("END : ", nextEventfromCalendar.end)
            #print ("NOW : ", now)
            nextStartTime = nextEventfromCalendar.start - now
            if(nextEventfromCalendar.start < now):
                if(logNoEventDisplayed == False):
                    print("No event Scheduled")
                    logNoEventDisplayed = True
		#only retrieve next event if we are out of an ongoing event
		if(nextEventfromCalendar.end < now):    
		    getNextEventFromCalendar()
            else:
                #print("Next Start in :", nextStartTime)
		totalSeconds = nextStartTime.seconds
		hours, remainder = divmod(totalSeconds, 3600)
		minutes, seconds = divmod(remainder, 60)
		timeString = str(hours) + ":" + str(minutes) + ":" + str(seconds)
		mqttClient.publish("ECS/next_start", payload=str(timeString), qos=0, retain=True)

            
            if (now > nextEventfromCalendar.start) and  (now < nextEventfromCalendar.end): 
                nextEndTime = nextEventfromCalendar.end - now
		mqttClient.publish("ECS/next_start", payload="started", qos=0, retain=True)

                print("Next End in :", nextEndTime)
                if(ecsState == ECS_STATE_OFF):
                    ecsState = ECS_STATE_ON
                    if(nextEventfromCalendar.title == "HIGH"):
                        heatProfile = "HIGH"
                    elif (nextEventfromCalendar.title == "LOW"):
                        heatProfile = "LOW"
                    else:
                        heatProfile = "MEDIUM"
                    tempMsg = heatMgrMessage('ECS_STATE_CHANGE', ECS_STATE_ON, heatProfile)
                    heatMgrQueue.put(tempMsg)
                    print("Switching ECS STATE :", ecsState)

            elif(ecsState != ECS_STATE_OFF):
                ecsState = ECS_STATE_OFF
                tempMsg = heatMgrMessage('ECS_STATE_CHANGE', ECS_STATE_OFF)
                heatMgrQueue.put(tempMsg)
                print("Switching ECS STATE :", ecsState)
                logNextEndDisplayed = False
                logNoEventDisplayed = False

        else:
            if(ecsState == ECS_STATE_ON):
                logNoEventDisplayed = False
                ecsState = ECS_STATE_OFF
                tempMsg = heatMgrMessage('ECS_STATE_CHANGE', ECS_STATE_OFF)
                heatMgrQueue.put(tempMsg)
                print("Switching ECS STATE :", ecsState)

            if(logNoEventDisplayed == False):
                print("No event Scheduled")
                logNoEventDisplayed = True
        
        time.sleep(DELAY_BETWEEN_SCHED_CHECK)

  
#=========================================================================#
# Main...                                                                 #
#=========================================================================#  
def main():
    global calendarId
    config = ConfigParser.ConfigParser()
    config.read('myconf.conf')
    calendarId = config.get('Calendar', 'calendarId')
    mqttAddress = config.get('MQTT', 'mqttAddress')
    mqttPort = config.get('MQTT', 'mqttPort')
    global configWarningSender, configWarningRecipient, configSmtpLogin, configSmtpPassword
    
    configWarningSender     = config.get('EMAIL', 'warningSender')
    configWarningRecipient  = config.get('EMAIL', 'warningRecipient')
    configSmtpLogin         = config.get('EMAIL', 'smtpLogin')
    configSmtpPassword      = config.get('EMAIL', 'smtpPassword')  
    
    global heatMgrQueue
    heatMgrQueue = Queue()
    
    mqttClient = mqtt.Client(userdata=heatMgrQueue)
    print ("heatMgrQueue : ") 
    print (heatMgrQueue)
    mqttClient.on_connect = on_connect
    mqttClient.on_message = on_message
    mqttClient.connect(mqttAddress, mqttPort)
    mqttClient.loop_start()
    mqttClient.subscribe("ECS/temp1")
    mqttClient.subscribe("ECS/temp2")
    mqttClient.subscribe("ECS/force")
    mqttClient.subscribe("ECS/calendar_update")
    
    CalendarMonitorThread = Thread(target=calendarMonitor, args=(calendarId,))
    CalendarMonitorThread.start()
    EcsStateSchedulerThread = Thread(target=ecsStateScheduler, args=(heatMgrQueue,mqttClient,))
    EcsStateSchedulerThread.start() 
    HeatManagerThread = Thread(target=heatManager, args=(heatMgrQueue,mqttClient,))    
    HeatManagerThread.start() 
  #  MqttLoopThread = Thread(target=mqttLoop, args=(heatMgrQueue,mqttClient,))    
  #  MqttLoopThread.start() 
    while 1:
        time.sleep(1000)

if __name__ == '__main__':
    main()
