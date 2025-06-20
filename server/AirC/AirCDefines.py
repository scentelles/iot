#from AirCManager import *

DEFAULT_TARGET_TEMP = 22

SWITCH_ON_TIMER_LIMIT  = 600 #10 minutes
SWITCH_OFF_TIMER_LIMIT = 300 #5 minutes
DEBUG = False

pingTime = 0

STATE_INIT = 1
STATE_WAIT_ESP_INIT = 2
STATE_READY = 3


MAX_OPEN_ANGLE = 90

CHAMBRE1 = "CHAMBRE1"
CHAMBRE2 = "CHAMBRE2"
CHAMBRE3 = "CHAMBRE3"
DREAMROOM = "DREAMROOM"
SALON = "SALON"
ETAGE = "ETAGE"


AC_MODE_OFF  = 1
AC_MODE_COOL = 2
AC_MODE_HEAT = 3
AC_MODE_FAN  = 4
AC_MODE_DRY  = 5



roomList = {}


MQTT_ESP_AERAULIC_STATE = "AC/ESP/AERAULIC_STATE"
MQTT_ESP_HOST_INIT_REQUEST = "AC/ESP/HOST_INIT_REQUEST"
MQTT_ESP_INIT_DONE = "AC/ESP/INIT_DONE"
MQTT_ESP_INIT_SERVO_DONE = "AC/ESP/INIT_SERVO_DONE"
MQTT_ESP_INIT_STARTED = "AC/ESP/INIT_STARTED"
MQTT_ESP_PONG =  "AC/ESP/PONG"
MQTT_ESP_PING =  "AC/ESP/PING"
MQTT_ESP_GREE_AMBIANT_TEMP =  "AC/GREE/corestatus/ambianttemp"
MQTT_HA_STARTED = "HA/start"

MQTT_AC_MODE = "AC/mode" #used to on/off from HA
MQTT_AC_MODE_OFF = b'0'
MQTT_AC_MODE_HEAT = b'1'
MQTT_AC_MODE_COOL = b'2'
MQTT_AC_MODE_FAN  = b'3'
MQTT_AC_MODE_DRY  = b'4'


MQTT_AC_TURBO_FORCED = "AC/turboforced"

MQTT_ADDRESS = {}
MQTT_ADDRESS[CHAMBRE1]  = "zigbee2mqtt/ZB_temp_Ch1"
MQTT_ADDRESS[CHAMBRE2] 	= "zigbee2mqtt/ZB_temp_Ch2"
MQTT_ADDRESS[CHAMBRE3] 	= "zigbee2mqtt/ZB_temp_Ch3"
MQTT_ADDRESS[DREAMROOM] = "zigbee2mqtt/ZB_temp_DR" 
MQTT_ADDRESS[SALON] 	= "zigbee2mqtt/ZB_temp_Salon"
MQTT_ADDRESS[ETAGE] 	= "zigbee2mqtt/ZB_temp_Etage"

MQTT_TO_ROOM = {}
MQTT_TO_ROOM[MQTT_ADDRESS[CHAMBRE1]] 	= CHAMBRE1
MQTT_TO_ROOM[MQTT_ADDRESS[CHAMBRE2]] 	= CHAMBRE2
MQTT_TO_ROOM[MQTT_ADDRESS[CHAMBRE3]]	= CHAMBRE3
MQTT_TO_ROOM[MQTT_ADDRESS[DREAMROOM]] 	= DREAMROOM
MQTT_TO_ROOM[MQTT_ADDRESS[SALON]] 	= SALON
MQTT_TO_ROOM[MQTT_ADDRESS[ETAGE]] 	= ETAGE

SAFETY_ROOM_CHANNEL = "SALON"

MQTT_GREE_PREFIX = "AC/GREE"
MQTT_SUFFIX_TARGETTEMP = "targettemp"
MQTT_SUFFIX_AC_STATE = "state"
MQTT_PREFIX = "AC"
AC_STATE_ON = 2
AC_STATE_OFF = 1

GREE_FANSPEED_AUTO 	 = 0
GREE_FANSPEED_LOW 	 = 1
GREE_FANSPEED_MEDIUMLOW  = 2
GREE_FANSPEED_MEDIUM     = 3
GREE_FANSPEED_MEDIUMHIGH = 4
GREE_FANSPEED_HIGH 	 = 5	
GREE_FANSPEED_TURBO 	 = 6

def getRoomFromAddress(address):
    for name in roomList:
        if(address.find(name) != -1):
            #print("Found room in address : " + name + "\n")
            return name
	
