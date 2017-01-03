#include <ESP8266WiFi.h>

const char* ssid     = "ESP8266";
const char* password = "audiowifi";

#define BOX_STATUS_PIN      D2    
#define AMPLIFIER_RELAY_PIN D5
#define TV_RELAY_PIN        D6
#define DELAY_BETWEEN_TOGGLE 10000
#define DELAY_BETWEEN_BOX_CHECK 10
 
#define BOX_OFF 1
#define BOX_ON  2

int boxStatus = BOX_OFF;
unsigned long lastSwitch = 0;

void setup() {
  Serial.begin(115200);
  pinMode(BOX_STATUS_PIN, INPUT);
  pinMode(AMPLIFIER_RELAY_PIN, OUTPUT);
  digitalWrite(AMPLIFIER_RELAY_PIN, HIGH);
  pinMode(TV_RELAY_PIN, OUTPUT);
  digitalWrite(TV_RELAY_PIN, HIGH);  
  
  delay(10);
}

bool checkToggleTooFast(){
  Serial.print("now :");
  Serial.println(millis());
  Serial.print("last :");
  Serial.println(lastSwitch);  
   if(millis() - lastSwitch < DELAY_BETWEEN_TOGGLE)
      return true;
   else
      return false;
      
}
void switchAllOn(){
  if(checkToggleTooFast()){
    Serial.println("Skipping switch because toggling too fast");
    return;
  }
    
  Serial.println("Switching all ON");
  digitalWrite(AMPLIFIER_RELAY_PIN, LOW);
  digitalWrite(TV_RELAY_PIN, LOW);  
  boxStatus = BOX_ON;
  lastSwitch = millis();

}
void switchAllOff(){
  if(checkToggleTooFast()){
    Serial.println("Skipping switch because toggling too fast");
    return;
  }
  Serial.println("Switching all OFF");
  digitalWrite(AMPLIFIER_RELAY_PIN, HIGH);
  digitalWrite(TV_RELAY_PIN, HIGH);  
  boxStatus = BOX_OFF;
  lastSwitch = millis();
}


void loop() {
  // put your main code here, to run repeatedly:

  int boxStatusPin = digitalRead(BOX_STATUS_PIN);


  if((boxStatusPin == 1) and (boxStatus == BOX_OFF)){
        switchAllOn();
  }
  if((boxStatusPin == 0) and (boxStatus == BOX_ON)){
        switchAllOff();
  }
  
  
  delay(DELAY_BETWEEN_BOX_CHECK);


}
