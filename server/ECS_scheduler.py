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

from enum import Enum

#import paho.mqtt.client as mqtt
from threading import Thread
from Queue import Queue

#TODO unused
class EcsCommand(Enum):
    OFF         = 1
    ON          = 2
    
    
class EcsHeatProfile(Enum):
    LOW         = 1
    MEDIUM      = 2
    HIGH        = 3



    
#defines
ECS_STATE_OFF = "OFF"
ECS_STATE_ON = "ON"


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
DELAY_BETWEEN_AGENDA_CHECK  = 10  # in multiples of SCHED check
NB_EVENTS_TO_GET_FROM_CALENDAR  = 1



class ThermoEvent:
        def __init__(self, title, value, start, end):
                self.title = title
                self.value = value
                self.start = start
                self.end = end

# The callback for when the client receives a CONNACK response from the server.
def on_connect(client, userdata, flags, rc):
    print("Connected with result code "+str(rc) + "\n")

def on_message(client, userdata, msg):
    print(msg.topic+" "+str(msg.payload) + "\n")
    



def get_date_object(date_string):
  return iso8601.parse_date(date_string)

def get_date_string(date_object):
  return rfc3339.rfc3339(date_object)  



    
		
#=========================================================================#
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

   
def getTargetTemperature(profile):
    if (profile == EcsHeatProfile.HIGH):
        return 55
    elif (profile == EcsHeatProfile.MEDIUM):
        return 50
    elif (profile == EcsHeatProfile.LOW):
        return 45
    else:
        return 50
        
   
#=========================================================================#
# Heat manager thread body                                                #
# Periodically checks for variable state changes :                        # 
#       - ecsState                                                        #
#       - heatProfile (used to set target temperature)                    #
#       - temperature(s) from the ECS sensor(s)                           #
#=========================================================================#
def heatManager(msqQueue, mqttClient):
    ecsState = ECS_STATE_OFF 
    ecsStateForced = False
    
    ecsTemperature = 0
    heatProfile  = EcsHeatProfile.MEDIUM
    
    
    while True:
        print ("HeatManager waiting for message")
        msg = msqQueue.get()
        msgType = msg[0]
        msgValue = msg[1]
        print ("HeatManager waking up. message received")
        #process messages

        if(msgType == "ECS_STATE_CHANGE"):
            if (ecsStateForced is False):
                if (msgValue == ECS_STATE_OFF):
                    print("Heat Manager : turning ECS OFF")
                    ecsState = ECS_STATE_OFF 
                    #mqttClient.publish("ECS/state", payload='1', qos=0, retain=False)

                elif (msgValue == ECS_STATE_ON):
                    heatProfile = msg[2]
                    ecsState = ECS_STATE_ON
                    if(ecsTemperature < getTargetTemperature(heatProfile)):
                        print("Heat Manager : turning ECS ON")
                        #mqttClient.publish("ECS/state", payload='2', qos=0, retain=False)
                    else:
                        print("Heat Manager : No ECS ON despite calendar, due to target temperature already reached")
                else: 
                    print("Heat Manager : Error : unknown EcsState %s in received message" % msgValue)
        elif(msgType == "ECS_TEMPERATURE"):
            ecsTemperature = msgValue
            print ("updating temperature : %i", msgValue)
            #Check against temperature target when ECS is ON and not in forced mode
            if ((ecsState == ECS_STATE_ON) and (ecsStateForced is False)):
                if (ecsTemperature < getTargetTemperature(heatProfile)):
                    print("Heat Manager : Switching ECS OFF despite calendar, due to target temperature reached")
                    ecsState = ECS_STATE_OFF
                    #mqttClient.publish("ECS/state", payload='1', qos=0, retain=False)
                    
                    
        elif(msgType == "ECS_FORCE"):   
            if (msgValue == ECS_STATE_OFF):
                print("Heat Manager : Forcing ECS OFF") 
                ecsStateForced = True
                if (ecsState == ECS_STATE_ON):
                    print("\tHeat Manager : Switching ECS OFF") 
                    #mqttClient.publish("ECS/state", payload='1', qos=0, retain=False)
                    ecsState = ECS_STATE_OFF
            elif (msgValue == ECS_STATE_ON):
                print("Heat Manager : Forcing ECS ON") 
                ecsStateForced = True
                if (ecsState == ECS_STATE_OFF):
                    print("\tHeat Manager : Switching ECS ON") 
                    #mqttClient.publish("ECS/state", payload='2', qos=0, retain=False)
                    ecsState = ECS_STATE_ON
 
            else:  
                print("Heat Manager : Unknown message value %s " % msgValue)
        else:
            print("Heat Manager : Unknown message type %s " % msgType)

def calendarMonitor(calendarId):
    global nextEventfromCalendar 
    while True:
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
              
        time.sleep(DELAY_BETWEEN_AGENDA_CHECK)

def mqttLoop(heatMgrQueue, mqttClient):
    count = 0
    while True:
        heatMgrQueue.put(("ECS_TEMPERATURE" , 22 + count))
        if(count == 30):
            heatMgrQueue.put(("ECS_FORCE" , ECS_STATE_OFF))
        if(count == 75):
            heatMgrQueue.put(("ECS_FORCE" , ECS_STATE_ON))
        count += 1
        time.sleep(1)
        

def ecsStateScheduler(heatMgrQueue):
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
            else:
                print("Next Start in :", nextStartTime)

            
            if (now > nextEventfromCalendar.start) and  (now < nextEventfromCalendar.end): 
                nextEndTime = nextEventfromCalendar.end - now
                print("Next End in :", nextEndTime)
                if(ecsState == ECS_STATE_OFF):
                    ecsState = ECS_STATE_ON
                    if(nextEventfromCalendar.title == "HIGH"):
                        heatProfile = "HIGH"
                    elif (nextEventfromCalendar.title == "LOW"):
                        heatProfile = "LOW"
                    else:
                        heatProfile = "MEDIUM"
                    heatMgrQueue.put(("ECS_STATE_CHANGE", ECS_STATE_ON, heatProfile))
                    print("Switching ECS STATE :", ecsState)

            elif(ecsState != ECS_STATE_OFF):
                ecsState = ECS_STATE_OFF
                heatMgrQueue.put(("ECS_STATE_CHANGE", ECS_STATE_OFF))
                print("Switching ECS STATE :", ecsState)
                logNextEndDisplayed = False
                logNoEventDisplayed = False

        else:
            if(ecsState == ECS_STATE_ON):
                logNoEventDisplayed = False
                ecsState = ECS_STATE_OFF
                heatMgrQueue.put(("ECS_STATE_CHANGE", ECS_STATE_OFF))
                print("Switching ECS STATE :", ecsState)

            if(logNoEventDisplayed == False):
                print("No event Scheduled")
                logNoEventDisplayed = True
        
        time.sleep(DELAY_BETWEEN_SCHED_CHECK)

        
def main():
    config = ConfigParser.ConfigParser()
    config.read('myconf.conf')
    calendarId = config.get('Calendar', 'calendarId')
    mqttAddress = config.get('MQTT', 'mqttAddress')
  
    heatMgrQueue = Queue()
    
    mqttClient = ""
 #   mqttClient = mqtt.Client()
 #   mqttClient.on_connect = on_connect
 #   mqttClient.on_message = on_message
 #   mqttClient.connect(MQTT_ADDRESS)
 #   mqttClient.loop_start()

    CalendarMonitorThread = Thread(target=calendarMonitor, args=(calendarId,))
    CalendarMonitorThread.start()
    EcsStateSchedulerThread = Thread(target=ecsStateScheduler, args=(heatMgrQueue,))
    EcsStateSchedulerThread.start() 
    HeatManagerThread = Thread(target=heatManager, args=(heatMgrQueue,mqttClient,))    
    HeatManagerThread.start() 
    MqttLoopThread = Thread(target=mqttLoop, args=(heatMgrQueue,mqttClient,))    
    MqttLoopThread.start() 

if __name__ == '__main__':
    main()
