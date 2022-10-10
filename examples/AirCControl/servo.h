

#ifdef LOLIN
  #define SERVO1_TURNL_PIN        19
  #define SERVO1_TURNR_PIN        23 
  #define SERVO2_TURNL_PIN        18
  #define SERVO2_TURNR_PIN        17 
  #define SERVO3_TURNL_PIN        0
  #define SERVO3_TURNR_PIN        2 
  #define SERVO4_TURNL_PIN        13
  #define SERVO4_TURNR_PIN        15 
  
  #define SERVO_MASTER1_TURNL_PIN        12
  #define SERVO_MASTER1_TURNR_PIN        14 
  #define SERVO_MASTER2_TURNL_PIN        27
  #define SERVO_MASTER2_TURNR_PIN        26 
  #define SERVO_MASTER3_TURNL_PIN        33
  #define SERVO_MASTER3_TURNR_PIN        32 
#else //for Wemos mini 32
  #define SERVO1_TURNL_PIN        25
  #define SERVO1_TURNR_PIN        27 
  #define SERVO2_TURNL_PIN        32
  #define SERVO2_TURNR_PIN        4 
  #define SERVO3_TURNL_PIN        22
  #define SERVO3_TURNR_PIN        21 
  #define SERVO4_TURNL_PIN        16
  #define SERVO4_TURNR_PIN        17 
  
  #define SERVO_MASTER1_TURNL_PIN        5
  #define SERVO_MASTER1_TURNR_PIN        23 
  #define SERVO_MASTER2_TURNL_PIN        19
  #define SERVO_MASTER2_TURNR_PIN        18 
  #define SERVO_MASTER3_TURNL_PIN        33
  #define SERVO_MASTER3_TURNR_PIN        35 
#endif

#define SERVO_CHAMBRE1  0
#define SERVO_CHAMBRE2  1
#define SERVO_CHAMBRE3  2
#define SERVO_DREAMROOM 3
#define SERVO_ETAGE     4
#define SERVO_SALON     5
#define SERVO_MASTER2   6



#define NB_SECONDS_FOR_90_DEGREES 78
#define TIMING_PER_DEGREE (1000 * NB_SECONDS_FOR_90_DEGREES) / 90


#define NB_SERVO  7

String ID_TO_ROOM[NB_SERVO];
int ID_TO_SERVOL[NB_SERVO];
int ID_TO_SERVOR[NB_SERVO];

int positionLoopStartTime[NB_SERVO];

void logServo(String msg)
{
 // Serial.print(msg);
}
void logServoCR(String msg)
{
 // Serial.println(msg);
}
void initPin(int pinId)
{
  pinMode(pinId, OUTPUT);
  digitalWrite(pinId, LOW);
}

void initServoPins()
{
  initPin(SERVO1_TURNL_PIN);
  initPin(SERVO1_TURNR_PIN);
  initPin(SERVO2_TURNL_PIN);
  initPin(SERVO2_TURNR_PIN);
  initPin(SERVO3_TURNL_PIN);
  initPin(SERVO3_TURNR_PIN);
  initPin(SERVO4_TURNL_PIN);
  initPin(SERVO4_TURNR_PIN);

  initPin(SERVO_MASTER1_TURNL_PIN);
  initPin(SERVO_MASTER1_TURNR_PIN); 
  initPin(SERVO_MASTER2_TURNL_PIN);
  initPin(SERVO_MASTER2_TURNR_PIN);
  initPin(SERVO_MASTER3_TURNL_PIN);
  initPin(SERVO_MASTER3_TURNR_PIN);
  
  delay(10);

  ID_TO_ROOM[SERVO_CHAMBRE1]  = "CHAMBRE1";
  ID_TO_ROOM[SERVO_CHAMBRE2]  = "CHAMBRE2";
  ID_TO_ROOM[SERVO_CHAMBRE3]  = "CHAMBRE3";
  ID_TO_ROOM[SERVO_DREAMROOM] = "DREAMROOM";
  ID_TO_ROOM[SERVO_ETAGE]     = "ETAGE";
  ID_TO_ROOM[SERVO_SALON]     = "SALON";
  ID_TO_ROOM[SERVO_MASTER2]   = "MASTER2";

  ID_TO_SERVOL[SERVO_CHAMBRE1]  = SERVO1_TURNL_PIN;
  ID_TO_SERVOL[SERVO_CHAMBRE2]  = SERVO2_TURNL_PIN;
  ID_TO_SERVOL[SERVO_CHAMBRE3]  = SERVO3_TURNL_PIN;
  ID_TO_SERVOL[SERVO_DREAMROOM] = SERVO4_TURNL_PIN;
  ID_TO_SERVOL[SERVO_ETAGE]     = SERVO_MASTER1_TURNL_PIN;
  ID_TO_SERVOL[SERVO_SALON]     = SERVO_MASTER3_TURNL_PIN;
  ID_TO_SERVOL[SERVO_MASTER2]   = SERVO_MASTER2_TURNL_PIN;

  ID_TO_SERVOR[SERVO_CHAMBRE1]  = SERVO1_TURNR_PIN;
  ID_TO_SERVOR[SERVO_CHAMBRE2]  = SERVO2_TURNR_PIN;
  ID_TO_SERVOR[SERVO_CHAMBRE3]  = SERVO3_TURNR_PIN;
  ID_TO_SERVOR[SERVO_DREAMROOM] = SERVO4_TURNR_PIN;
  ID_TO_SERVOR[SERVO_ETAGE]     = SERVO_MASTER1_TURNR_PIN;
  ID_TO_SERVOR[SERVO_SALON]     = SERVO_MASTER3_TURNR_PIN;
  ID_TO_SERVOR[SERVO_MASTER2]   = SERVO_MASTER2_TURNR_PIN;

  
}
