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
        def __init__(self, name, value, start, end):
                self.name = name
                self.value = value
                self.start = start
                self.end = end

def datetime_to_336(dt):
        r = 0
        r += 48 * dt.weekday()
        r += 2 * dt.hour
        if dt.minute >= 30:
                r += 1
        return r
		
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


def main():
    """Shows basic usage of the Google Calendar API.

    Creates a Google Calendar API service object and outputs a list of the next
    10 events on the user's calendar.
    """
	
	#Custom part
    config = ConfigParser.ConfigParser()
    config.read('myconf.conf')

    calendarId = config.get('Calendar', 'calendarId')
	
	#End of custom
	
    
    while(1):
        events = getEventsFromCalendar(calendarId)
        if not events:
            print('No upcoming events found.')
        else:
            thermoEvents = []
            count = 0
            
            for event in events:
                title = event['summary']
                content = ""
                start = get_date_object(event['start'].get('dateTime'))
                end = get_date_object(event['end'].get('dateTime'))
                thermoEvents.append(ThermoEvent(title, content, start, end))

            while(count < DELAY_BETWEEN_AGENDA_CHECK):
                count += 1
                parisTz = pytz.timezone('Europe/Paris')
                now = parisTz.localize(datetime.datetime.now())

                delta = thermoEvents[0].start - now
                print ("DELTA with next start event: ", delta)
                if (now > thermoEvents[0].start) and  (now < thermoEvents[0].end):  
                # call to your scheduled task goes here
                    print("\nScheduling task !!!!!!!!!!\n")
                
                time.sleep(DELAY_BETWEEN_SCHED_CHECK)
    	


if __name__ == '__main__':
    main()