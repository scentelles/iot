#if defined(ESP8266)
  #include <ESP8266WiFi.h> //TODO : should not need the entire include just for Strings and serial
#else
  #include <WiFi.h>
#endif
#include <EEPROM.h>



//TODO : can become parameter of the constructor
#define EEPROM_FIELD_SIZE 32
#define EEPROM_NB_FIELD   5

#define EEPROM_FORMATTED 'F'


class PersistentConfig{
    private :
    char  eepromArray[EEPROM_NB_FIELD][EEPROM_FIELD_SIZE];
    
    public:
    PersistentConfig();
    void stageWriteField(int index, String str);
    void commit();
    void readAll();
    String readField(int index);
    
};