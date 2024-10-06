//#define TELNET_DEBUG
//#define MY_SERIAL_DEBUG
#define LOLIN


#ifdef LOLIN
  #define UART_RX 25
  #define UART_TX 16
  #define UART_RXTX 4
#else //for Wemos mini 32
  #define UART_RX 26
  #define UART_TX 0
  #define UART_RXTX 2
#endif

#define MODBUS_SLAVE_ID 1


#define NB_HREGS 84
String hregs[NB_HREGS];
#define NB_COILS 216
String coils[NB_COILS];

ModbusRTU mb;
bool resultCoils[NB_COILS];
uint16_t resultHregs[NB_HREGS];



int greeCurrentValuePower = 0;
int greeCurrentValueAmbiantTemp = 0;      
int greeCurrentValueMode = 0;      
int greeCurrentValueFanSpeed = 0;      
int greeCurrentValueTemperature = 0;
int greeCurrentValueTempLowerLimitNrj = 0;
int greeCurrentValueTempUpperLimitNrj = 0;
int greeCurrentValueSleepMode = 0;
int greeCurrentValueOutdoorTemp = 0;
int greeCurrentValueAirReturnTemp = 0;

int greeCurrentValuePowerNew = 0;
int greeCurrentValueAmbiantTempNew = 0;      
int greeCurrentValueModeNew = 0;      
int greeCurrentValueFanSpeedNew = 0;      
int greeCurrentValueTemperatureNew = 0;
int greeCurrentValueTempLowerLimitNrjNew = 0;
int greeCurrentValueTempUpperLimitNrjNew = 0;
int greeCurrentValueSleepModeNew = 0;
int greeCurrentValueOutdoorTempNew = 0;
int greeCurrentValueAirReturnTempNew = 0;

bool greeCurrentValueTurbo = false;      
bool greeCurrentValueSilent = false;  
bool greeCurrentValueNrjSaving = false;  
bool greeCurrentValueNrjSavingCool = false;  
bool greeCurrentValueNrjSavingHeat = false;  
bool greeCurrentValueOutdoorFan = false;   
bool greeCurrentValueCompressor = false;    
bool greeCurrentValueSystemDefrosting = false;  

bool greeCurrentValueTurboNew = false;      
bool greeCurrentValueSilentNew = false;  
bool greeCurrentValueNrjSavingNew = false;  
bool greeCurrentValueNrjSavingCoolNew = false;  
bool greeCurrentValueNrjSavingHeatNew = false;  
bool greeCurrentValueOutdoorFanNew = false;   
bool greeCurrentValueCompressorNew = false;    
bool greeCurrentValueSystemDefrostingNew = false;  

//================== HREGS ====================
#define GREE_HREG_RW_ONOFF 2
#define GREE_ON 0xAA
#define GREE_OFF 0x55

#define GREE_HREG_R_AMBIANT_TEMP       4
#define GREE_HREG_R_IDU_ADDRESS        5

#define GREE_HREG_RW_SET_MODE   17
greeSetMode(int greeMode)
{
    greeWriteHreg(GREE_HREG_RW_SET_MODE, greeMode);
}
void greeSetFanSpeed(short value)
{
    greeWriteHreg(GREE_HREG_RW_SET_FAN_SPEED, value);
}
void greeSetTemperature(int value)
{
    greeWriteHreg(GREE_HREG_RW_SET_TEMP, value);
}
void greeSetTemperatureLowerLimitNrj(int value)
{
    greeWriteHreg(GREE_HREG_RW_TEMP_LOWER_LIMIT_NRJ, value);
}
void greeSetTemperatureUpperLimitNrj(int value)
{
    greeWriteHreg(GREE_HREG_RW_TEMP_UPPER_LIMIT_NRJ, value);
}
void greeSetTurbo(bool value)
{
    greeWriteCoil(GREE_SWITCH_RW_TURBO, value);
}
void greeSetSilent(bool value)
{
    greeWriteCoil(GREE_SWITCH_RW_SILENT, value);
}
void greeSetNRJSaving(bool value)
{
    greeWriteCoil(GREE_SWITCH_RW_NRJ_SAVING, value);
    greeWriteCoil(GREE_SWITCH_RW_NRJ_SAVING_COOL, value);
    greeWriteCoil(GREE_SWITCH_RW_NRJ_SAVING_HEAT, value);    
}
