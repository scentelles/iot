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
import sched, time

import pytz
from enum import Enum

#import paho.mqtt.client as mqtt
from threading import Thread

mqttClient = 0



class EcsCommand(Enum):
    OFF         = 1
    ON          = 2
    
class EcsHeatProfile(Enum):
    LOW         = 1
    MEDIUM      = 2
    HIGH     = 3


MQTT_ADDRESS = "localhost"
    
ECS_STATE_OFF = "OFF"
ECS_STATE_ON = "ON"
#Globals
ecsState = ECS_STATE_OFF  
heatProfile  = EcsHeatProfile.MEDIUM
nextStartTime = ""
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

def heatManager():
    global ecsState
    global mqttClient
    previousEcsState = ecsState
    while True:
        if(ecsState != previousEcsState):
            if (ecsState == ECS_STATE_OFF):
                print("Heat Manager : turning ECS OFF")
                #mqttClient.publish("ECS/state", payload='1', qos=0, retain=False)

            else: 
                if (ecsState == ECS_STATE_ON):
                    print("Heat Manager : turning ECS ON")
                    #mqttClient.publish("ECS/state", payload='2', qos=0, retain=False)

                else: 
                    print("Heat Manager : Error : unknown EcsState")
        
        #if ON
        # compare with temperature target
        # if temerature reached, send OFF command
        
        
        previousEcsState = ecsState
        time.sleep(1)

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

def ecsStateScheduler():
    global ecsState
    logNoEventDisplayed     = False
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
                    print("Switching ECS STATE :", ecsState)
                    if(nextEventfromCalendar.title == "HIGH"):
                        heatProfile = "HIGH"
                    else:
                        if (nextEventfromCalendar.title == "LOW"):
                            heatProfile = "LOW"
                        else:
                            heatProfile = "MEDIUM"
            else:
                if(ecsState != ECS_STATE_OFF):
                    ecsState = ECS_STATE_OFF
                    print("Switching ECS STATE :", ecsState)
                    logNextEndDisplayed = False
                    logNoEventDisplayed = False

        else:
            if(ecsState == ECS_STATE_ON):
                logNoEventDisplayed = False
                ecsState = ECS_STATE_OFF
                print("Switching ECS STATE :", ecsState)

            if(logNoEventDisplayed == False):
                print("No event Scheduled")
                logNoEventDisplayed = True
        
        time.sleep(DELAY_BETWEEN_SCHED_CHECK)

        
def main():
    config = ConfigParser.ConfigParser()
    config.read('myconf.conf')
    calendarId = config.get('Calendar', 'calendarId')


    global mqttClient
 #   mqttClient = mqtt.Client()
 #   mqttClient.on_connect = on_connect
 #   mqttClient.on_message = on_message
 #   mqttClient.connect(MQTT_ADDRESS)
 #   mqttClient.loop_start()

    CalendarMonitorThread = Thread(target=calendarMonitor, args=(calendarId,))
    CalendarMonitorThread.start()
    EcsStateSchedulerThread = Thread(target=ecsStateScheduler, args=())
    EcsStateSchedulerThread.start() 
    HeatManagerThread = Thread(target=heatManager, args=())    
    HeatManagerThread.start() 


if __name__ == '__main__':
    main()
