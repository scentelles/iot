#include "PersistentConfig.h"

PersistentConfig::PersistentConfig(){
    EEPROM.begin(512);
    //Read last char to check if EEPROM already formatted
    char tmpChar = (char)EEPROM.read(511);
    if(tmpChar != EEPROM_FORMATTED){
        Serial.println("EEPROM not formatted. Proceeding to format EEPROM");
        for (int addr = 0; addr < 512; addr++){
            EEPROM.write(addr, 0);
        }
        EEPROM.write(511, EEPROM_FORMATTED);
    }
        
    
}
void PersistentConfig::stageWriteField(int index, String str){
    
    if(index < EEPROM_NB_FIELD)
        strcpy(&eepromArray[index][0], str.c_str());
    else
        Serial.println("ERROR : accessing out of bound field in EEPROM staging array");
    
}

void PersistentConfig::commit(){
      for (int addr = 0; addr < sizeof(eepromArray); addr++){
          EEPROM.write(addr, ((char*)eepromArray)[addr]);
      }
      EEPROM.commit();
      Serial.println("EEPROM updated");
}

void PersistentConfig::readAll(){
          
          
          
    for (int addr = 0; addr < sizeof(eepromArray); addr++){
          char tmpChar = (char)EEPROM.read(addr);
          ((char*)eepromArray)[addr] = tmpChar;
    }
}

String PersistentConfig::readField(int index){
    
    if(index < EEPROM_NB_FIELD){
        Serial.print("Field read :");
        char * tmpChar = eepromArray[index];
        Serial.println(tmpChar);
        
        return String(tmpChar);
    }
    else 
        return "ERROR : index out of bound";
}