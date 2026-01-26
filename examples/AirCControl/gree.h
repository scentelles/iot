//#define TELNET_DEBUG
//#define MY_SERIAL_DEBUG
#define LOLIN
//#define WEMOSMINI
//#define ESP32_DEVKIT

#ifdef LOLIN
  #define UART_RX 25
  #define UART_TX 16
  #define UART_RXTX 4
#endif
#ifdef WEMOSMINI 
  #define UART_RX 26
  #define UART_TX 0
  #define UART_RXTX 2
#endif
#ifdef ESP32_DEVKIT 
  #define UART_RX 33
  #define UART_TX 18
  #define UART_RXTX 5
#endif

#define MODBUS_SLAVE_ID 1

extern bool enqueueMqttPublish(const char * leafTopic, const char * payload);


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
#define GREE_MODE_COOL          1
#define GREE_MODE_HEAT          2
#define GREE_MODE_DRY           3
#define GREE_MODE_FAN           4
#define GREE_MODE_AUTO          5

#define GREE_HREG_RW_SET_FAN_SPEED      19
#define GREE_HREG_RW_SET_TEMP           20
#define GREE_HREG_RW_UP_DOWN_SWING      22
#define GREE_HREG_RW_L_R_SWING          23
#define GREE_HREG_RW_FRESH_AIR_VALVE    24
#define GREE_HREG_RW_SLEEP_MODE         25
#define GREE_HREG_RW_CLEAN_FUNCTION     34
#define GREE_HREG_RW_TEMP_LOWER_LIMIT_NRJ  35
#define GREE_HREG_RW_TEMP_UPPER_LIMIT_NRJ  36  //mismatch with sleep
#define GREE_HREG_R_AMBIANT_TEMP_SELECT   39
#define GREE_AMBIANT_AIR_RETURN    1
#define GREE_AMBIANT_WIRED_CONTROLLER  2
#define GREE_AMBIANT_MIXED         3

#define GREE_HREG_R_OUTDOOR_TEMP        49
#define GREE_HREG_R_DRED                77
#define GREE_HREG_R_AIR_RETURN_TEMP     82
#define GREE_HREG_R_LIGHT_BOARD_TEMP    83

void initHregs()
{
  for(int i = 0; i < NB_HREGS; i++)
  {
    hregs[i] = "RESERVED";
  }

  hregs[0] = "FIRST HREG";
  hregs[2] = "GREE_HREG_RW_ONOFF";

  hregs[4] = "GREE_HREG_R_AMBIANT_TEMP";
  hregs[5] = "GREE_HREG_R_IDU_ADDRESS";
  hregs[17] = "GREE_HREG_RW_SET_MODE";
  hregs[19] = "GREE_HREG_RW_SET_FAN_SPEED";
  hregs[20] = "GREE_HREG_RW_SET_TEMP";
  hregs[22] = "GREE_HREG_RW_UP_DOWN_SWING";
  hregs[23] = "GREE_HREG_RW_L_R_SWING";
  hregs[24] = "GREE_HREG_RW_FRESH_AIR_VALVE";
  hregs[25] = "GREE_HREG_RW_SLEEP_MODE";
  hregs[34] = "GREE_HREG_RW_CLEAN_FUNCTION";
  hregs[35] = "GREE_HREG_RW_TEMP_LOWER_LIMIT_NRJ";
  hregs[36] = "GREE_HREG_RW_TEMP_UPPER_LIMIT_NRJ";   //seems there is a mismatch in the doc. between sleep mode and fresh air valve
  hregs[39] = "GREE_HREG_R_AMBIANT_TEMP_SELECT"; 
  hregs[49] = "GREE_HREG_R_OUTDOOR_TEMP";
  hregs[77] = "GREE_HREG_R_DRED";
  hregs[82] = "GREE_HREG_R_AIR_RETURN_TEMP";
  hregs[83] = "GREE_HREG_R_LIGHT_BOARD_TEMP";
}


//==================== COILS

#define GREE_STATUS_R_ODU      8
#define GREE_STATUS_R_MASTER_WIRED_CONTROLLER 9
#define GREE_STATUS_R_SLAVE_WIRED_CONTROLLER 13
#define GREE_SWITCH_RW_REMOTE_LOCK 17
#define GREE_SWITCH_RW_REMOTE_TEMP_SHIELD1 18
#define GREE_SWITCH_RW_REMOTE_TEMP_SHIELD2 19
#define GREE_SWITCH_RW_REMOTE_ONOFF_SHIELD 20
#define GREE_SWITCH_RW_REMOTE_NRJ_SHIELD   21
#define GREE_SWITCH_RW_ABSENCE_MODE        24
#define GREE_SWITCH_RW_HEALTHY             25
#define GREE_SWITCH_RW_NRJ_SAVING          26
#define GREE_SWITCH_RW_TURBO               27
#define GREE_SWITCH_RW_E_HEATING_PERM      28
#define GREE_SWITCH_RW_X_FAN               29
#define GREE_SWITCH_RW_SILENT              30
#define GREE_SWITCH_RW_LOW_TEMP_DRY        31
#define GREE_SWITCH_RW_NRJ_SAVING_COOL     32
#define GREE_SWITCH_RW_NRJ_SAVING_HEAT     33
#define GREE_SWITCH_RW_BUTTON_LOCK         34
#define GREE_SWITCH_RW_ON_OFF_MEMORY       35
#define GREE_SWITCH_RW_C_F                 36
#define GREE_STATUS_R_TIMER               40
#define GREE_STATUS_R_GATE_CONTROL        46
#define GREE_STATUS_R_BODY_SENSING        47
#define GREE_STATUS_R_TIMER_ON            49
#define GREE_STATUS_R_TIMER_OFF           50
#define GREE_STATUS_R_INDOOR_TEMP         51
#define GREE_STATUS_R_LOW_POWER_STANDBY   55
#define GREE_ERROR_R_WIRED_CONTROLLER_SENSOR     65
#define GREE_ERROR_R_WIRED_CONTROLLER_MEMORY     70
#define GREE_SWITCH_RW_CANCEL_TIMER        72
#define GREE_SWITCH_RW_UP_DOWN_SWING       74
#define GREE_STATUS_R_ELECTRIC_HEATING    91
#define GREE_STATUS_R_WATER_PUMP          92
#define GREE_STATUS_R_FRESH_AIR_VALVE     93
#define GREE_STATUS_R_COLD_PLASMA         94
#define GREE_STATUS_R_ERROR_OUTPUT        95
#define GREE_ERROR_R_INDOOR_EVAP_SENSOR   97
#define GREE_ERROR_R_AIR_RETURN_SENSOR    98
#define GREE_ERROR_R_LIGHTBOARD_TEMP_SENSOR    99
#define GREE_ERROR_R_WATER_OVERFLOW       100
#define GREE_ERROR_R_MEMORY_IDU           102
#define GREE_ERROR_R_JUMPER_CAP           104
#define GREE_ERROR_R_INDOOR_FAN           105
#define GREE_STATUS_R_NEED_CLEANING       108
#define GREE_STATUS_R_CARD_IN_OUT         112
#define GREE_ERROR_R_INDOOR_EVAPORATOR_TEMP_SENSOR       113
#define GREE_STATUS_R_STATIC_PRESSURE_TYPE       114
#define GREE_ERROR_R_COM_FAILURE_MASTER_WIRED_CTRL       120
#define GREE_ERROR_R_COM_FAILURE_SLAVE_WIRED_CTRL       121
#define GREE_ERROR_R_COM_FAILURE_ODU       122
#define GREE_STATUS_R_OUTDOOR_FAN          147
#define GREE_STATUS_R_4_WAY_VALVE          150
#define GREE_STATUS_R_COMPRESSOR           151
#define GREE_ERROR_R_COMP_DISCHARGE_TEMP_PROTECT        152
#define GREE_ERROR_R_FLUORINE_SHORTAGE_PROTECT          153
#define GREE_ERROR_R_DC_FAN_MOTOR_PROTECT               154
#define GREE_ERROR_R_4_WAY_VALVE_PROTECT                155
#define GREE_ERROR_R_OVER_POWER_PROTECT                 156
#define GREE_ERROR_R_OVER_LOAD_PROTECT                  157
#define GREE_ERROR_R_LOW_PRESSURE_PROTECT               158
#define GREE_ERROR_R_HIGH_PRESSURE_PROTECT              159
#define GREE_ERROR_R_EVAP_ANTI_FREEZE_PROTECT           160
#define GREE_ERROR_R_OUTDOOR_AMBIANT_TEMP_SENSOR        161
#define GREE_ERROR_R_DISCHARGE_TEMP_SENSOR              162
#define GREE_ERROR_R_CONDENSER_TEMP_SENSOR              163
#define GREE_ERROR_R_OUTDOOR_PIPE_TEMP_SENSOR           164
#define GREE_ERROR_R_JUMPER_CAP_2                       165
#define GREE_ERROR_R_ODU_MEMORY                         166
#define GREE_ERROR_R_COM_DRIVE                          174
#define GREE_STATUS_R_SAVE                              176
#define GREE_STATUS_R_COOL_ONLY_HEAT_PUMP_FLAG          177
#define GREE_STATUS_R_SYSTEM_DEFROSTING                 179
#define GREE_STATUS_R_LOW_POWER_CONSUMPTION             182
#define GREE_ERROR_R_AC_PHASE                           183
//goes up to 215

void initCoils()
{
  for(int i = 0; i < 215; i++)
  {
    coils[i] = "RESERVED";
  }
  coils[0] = "FIRST COIL";
  coils[8] = "GREE_STATUS_R_ODU";
  coils[9] = "GREE_STATUS_R_MASTER_WIRED_CONTROLLER";
  coils[13] = "GREE_STATUS_R_SLAVE_WIRED_CONTROLLER";
  coils[17] = "GREE_SWITCH_RW_REMOTE_LOCK";
  coils[18] = "GREE_SWITCH_RW_REMOTE_TEMP_SHIELD1";
  coils[19] = "GREE_SWITCH_RW_REMOTE_TEMP_SHIELD2";
  coils[20] = "GREE_SWITCH_RW_REMOTE_ONOFF_SHIELD";
  coils[21] = "GREE_SWITCH_RW_REMOTE_NRJ_SHIELD";
  coils[24] = "GREE_SWITCH_RW_ABSENCE_MODE";
  coils[25] = "GREE_SWITCH_RW_HEALTHY";
  coils[26] = "GREE_SWITCH_RW_NRJ_SAVING";
  coils[27] = "GREE_SWITCH_RW_TURBO";
  coils[28] = "GREE_SWITCH_RW_E_HEATING_PERM";
  coils[29] = "GREE_SWITCH_RW_X_FAN";
  coils[30] = "GREE_SWITCH_RW_SILENT";
  coils[31] = "GREE_SWITCH_RW_LOW_TEMP_DRY";
  coils[32] = "GREE_SWITCH_RW_NRJ_SAVING_COOL";
  coils[33] = "GREE_SWITCH_RW_NRJ_SAVING_HEAT";
  coils[34] = "GREE_SWITCH_RW_BUTTON_LOCK";
  coils[35] = "GREE_SWITCH_RW_ON_OFF_MEMORY";
  coils[36] = "GREE_SWITCH_RW_C_F";
  coils[40] = "GREE_STATUS_R_TIMER";
  coils[46] = "GREE_STATUS_R_GATE_CONTROL";
  coils[47] = "GREE_STATUS_R_BODY_SENSING";
  coils[49] = "GREE_STATUS_R_TIMER_ON";
  coils[50] = "GREE_STATUS_R_TIMER_OFF";
  coils[51] = "GREE_STATUS_R_INDOOR_TEMP";
  coils[55] = "GREE_STATUS_R_LOW_POWER_STANDBY";
  coils[65] = "GREE_ERROR_R_WIRED_CONTROLLER_SENSOR";
  coils[70] = "GREE_ERROR_R_WIRED_CONTROLLER_MEMORY";
  coils[72] = "GREE_SWITCH_RW_CANCEL_TIMER";
  coils[74] = "GREE_SWITCH_RW_UP_DOWN_SWING";
  coils[91] = "GREE_STATUS_R_ELECTRIC_HEATING";
  coils[92] = "GREE_STATUS_R_WATER_PUMP";
  coils[93] = "GREE_STATUS_R_FRESH_AIR_VALVE";
  coils[94] = "GREE_STATUS_R_COLD_PLASMA";
  coils[95] = "GREE_STATUS_R_ERROR_OUTPUT";
  coils[97] = "GREE_ERROR_R_INDOOR_EVAP_SENSOR";
  coils[98] = "GREE_ERROR_R_AIR_RETURN_SENSOR";
  coils[99] = "GREE_ERROR_R_LIGHTBOARD_TEMP_SENSOR";
  coils[100] = "GREE_ERROR_R_WATER_OVERFLOW";
  coils[102] = "GREE_ERROR_R_MEMORY_IDU";
  coils[104] = "GREE_ERROR_R_JUMPER_CAP";
  coils[105] = "GREE_ERROR_R_INDOOR_FAN";
  coils[108] = "GREE_STATUS_R_NEED_CLEANING";
  coils[112] = "GREE_STATUS_R_CARD_IN_OUT";
  coils[113] = "GREE_ERROR_R_INDOOR_EVAPORATOR_TEMP_SENSOR";
  coils[114] = "GREE_STATUS_R_STATIC_PRESSURE_TYPE";
  coils[120] = "GREE_ERROR_R_COM_FAILURE_MASTER_WIRED_CTRL";
  coils[121] = "GREE_ERROR_R_COM_FAILURE_SLAVE_WIRED_CTRL";
  coils[122] = "GREE_ERROR_R_COM_FAILURE_ODU";
  coils[147] = "GREE_STATUS_R_OUTDOOR_FAN";
  coils[150] = "GREE_STATUS_R_4_WAY_VALVE";
  coils[151] = "GREE_STATUS_R_COMPRESSOR";
  coils[152] = "GREE_ERROR_R_COMP_DISCHARGE_TEMP_PROTECT";
  coils[153] = "GREE_ERROR_R_FLUORINE_SHORTAGE_PROTECT";
  coils[154] = "GREE_ERROR_R_DC_FAN_MOTOR_PROTECT";
  coils[155] = "GREE_ERROR_R_4_WAY_VALVE_PROTECT";
  coils[156] = "GREE_ERROR_R_OVER_POWER_PROTECT";
  coils[157] = "GREE_ERROR_R_OVER_LOAD_PROTECT";
  coils[158] = "GREE_ERROR_R_LOW_PRESSURE_PROTECT";
  coils[159] = "GREE_ERROR_R_HIGH_PRESSURE_PROTECT";
  coils[160] = "GREE_ERROR_R_EVAP_ANTI_FREEZE_PROTECT";
  coils[161] = "GREE_ERROR_R_OUTDOOR_AMBIANT_TEMP_SENSOR";
  coils[162] = "GREE_ERROR_R_DISCHARGE_TEMP_SENSOR";
  coils[163] = "GREE_ERROR_R_CONDENSER_TEMP_SENSOR";
  coils[164] = "GREE_ERROR_R_OUTDOOR_PIPE_TEMP_SENSOR";
  coils[165] = "GREE_ERROR_R_JUMPER_CAP";
  coils[166] = "GREE_ERROR_R_ODU_MEMORY";
  coils[174] = "GREE_ERROR_R_COM_DRIVE";
  coils[176] = "GREE_STATUS_R_SAVE";
  coils[177] = "GREE_STATUS_R_COOL_ONLY_HEAT_PUMP_FLAG";
  coils[179] = "GREE_STATUS_R_SYSTEM_DEFROSTING";
  coils[183] = "GREE_ERROR_R_AC_PHASE";
}


#ifdef TELNET_DEBUG  
  RemoteDebug Debug;
#endif


void debugPrintln(const char * msg)
{
#ifdef TELNET_DEBUG    
    Debug.println(F(msg));
#else
    #ifdef MY_SERIAL_DEBUG
        Serial.println(msg);
    #else
        return;
    #endif
#endif
}
void debugPrint(const char * msg)
{
#ifdef TELNET_DEBUG    
    Debug.print(F(msg));
#else
    Serial.print(msg);
#endif
}

  
void readAllCoils()
{
   if (!mb.slave()) {
   
    int code = mb.readCoil(MODBUS_SLAVE_ID, 0, resultCoils, NB_COILS);
    debugPrintln("=======================\n read coils : ");
    debugPrintln(String("Code : " + String(code)).c_str());

    while(mb.slave()) { // Check if transaction is active
      mb.task();
      delay(10);
    }
    for(int i = 0; i < NB_COILS; i++)
    {
      if(coils[i] != "RESERVED")
      {
        debugPrint(String(i + String(" : ")).c_str());
        debugPrint(String(coils[i]+ String(" : ")).c_str());
        debugPrintln(String(resultCoils[i]).c_str());
      }
    }
    debugPrintln ("=========================");

  }

}



void readModbusSecondaryValues()
{

   if (!mb.slave()) 
   {
     mb.readCoil(MODBUS_SLAVE_ID, 0, resultCoils, GREE_STATUS_R_SYSTEM_DEFROSTING+1);
      while(mb.slave()) { // Check if transaction is active
      mb.task();
      delay(15);
      }

      greeCurrentValueTurboNew = resultCoils[GREE_SWITCH_RW_TURBO];      
      greeCurrentValueSilentNew = resultCoils[GREE_SWITCH_RW_SILENT];   
      greeCurrentValueNrjSaving = resultCoils[GREE_SWITCH_RW_NRJ_SAVING];  
    //  greeCurrentValueNrjSavingCool = resultCoils[GREE_SWITCH_RW_NRJ_SAVING_COOL];  
    //  greeCurrentValueNrjSavingHeat = resultCoils[GREE_SWITCH_RW_NRJ_SAVING_HEAT];  
      greeCurrentValueOutdoorFanNew = resultCoils[GREE_STATUS_R_OUTDOOR_FAN];  
      greeCurrentValueCompressorNew = resultCoils[GREE_STATUS_R_COMPRESSOR];  
      greeCurrentValueSystemDefrostingNew = resultCoils[GREE_STATUS_R_SYSTEM_DEFROSTING];  
                        
      debugPrintln(String("GREE TURBO        : " + String(greeCurrentValueTurboNew)).c_str());
      debugPrintln(String("GREE Silent        : " + String(greeCurrentValueSilentNew)).c_str());
      debugPrintln(String("GREE NRJ SAVING        : " + String(greeCurrentValueNrjSavingNew)).c_str());
    //  debugPrintln(String("GREE NRJ SAVING COOL        : " + String(greeCurrentValueNrjSavingCool)).c_str());
    //  debugPrintln(String("GREE NRJ SAVING HEAT        : " + String(greeCurrentValueNrjSavingHeat)).c_str());
      debugPrintln(String("GREE OUTDOOR FAN        : " + String(greeCurrentValueOutdoorFanNew)).c_str());
      debugPrintln(String("GREE COMPRESSOR        : " + String(greeCurrentValueCompressorNew)).c_str());      
      debugPrintln(String("GREE DEFROSTING        : " + String(greeCurrentValueSystemDefrostingNew)).c_str());     

      
   }
}

void sendModbusSecondaryValues()
{
      if(greeCurrentValueTurboNew != greeCurrentValueTurbo)
      {
        greeCurrentValueTurbo = greeCurrentValueTurboNew;
        enqueueMqttPublish("GREE/secondarystatus/turbo", String(greeCurrentValueTurbo).c_str());
      }
      if(greeCurrentValueSilentNew != greeCurrentValueSilent)
      {
        greeCurrentValueSilent = greeCurrentValueSilentNew;
        enqueueMqttPublish("GREE/secondarystatus/silent", String(greeCurrentValueSilent).c_str());
      }
      if(greeCurrentValueNrjSavingNew != greeCurrentValueNrjSaving)
      {
        greeCurrentValueNrjSaving = greeCurrentValueNrjSavingNew;
        enqueueMqttPublish("GREE/secondarystatus/nrjsaving", String(greeCurrentValueNrjSaving).c_str());
      }
      if(greeCurrentValueOutdoorFanNew != greeCurrentValueOutdoorFan)
      {
        greeCurrentValueOutdoorFan = greeCurrentValueOutdoorFanNew;
        enqueueMqttPublish("GREE/secondarystatus/outdoorfan", String(greeCurrentValueOutdoorFan).c_str());
      }
     // myMqtt->publishValue("GREE/secondarystatus/nrjsavingcool", String(greeCurrentValueNrjSavingCool).c_str());
     // myMqtt->publishValue("GREE/secondarystatus/nrjsavingheat", String(greeCurrentValueNrjSavingHeat).c_str());

      if(greeCurrentValueCompressorNew != greeCurrentValueCompressor)
      {
        greeCurrentValueCompressor = greeCurrentValueCompressorNew;
        enqueueMqttPublish("GREE/secondarystatus/compressor", String(greeCurrentValueCompressor).c_str());
      }
      if(greeCurrentValueSystemDefrostingNew != greeCurrentValueSystemDefrosting)
      {
        greeCurrentValueSystemDefrosting = greeCurrentValueSystemDefrostingNew;
        enqueueMqttPublish("GREE/secondarystatus/systemdefrosting", String(greeCurrentValueSystemDefrosting).c_str());
      }

}

void readModbusCoreValues()
{
    
   if (!mb.slave()) 
   {
      mb.readHreg(MODBUS_SLAVE_ID, 0, resultHregs, GREE_HREG_RW_SET_TEMP+1);
      while(mb.slave()) { // Check if transaction is active
      mb.task();
      delay(10);
      }

      greeCurrentValuePowerNew = resultHregs[GREE_HREG_RW_ONOFF];
      greeCurrentValueAmbiantTempNew = resultHregs[GREE_HREG_R_AMBIANT_TEMP];      
      greeCurrentValueModeNew = resultHregs[GREE_HREG_RW_SET_MODE];      
      greeCurrentValueFanSpeedNew = resultHregs[GREE_HREG_RW_SET_FAN_SPEED];      
      greeCurrentValueTemperatureNew = resultHregs[GREE_HREG_RW_SET_TEMP];
      //greeCurrentValueSleepModeNew = resultHregs[GREE_HREG_RW_SLEEP_MODE];
   } 
  

     if (!mb.slave()) 
     {
        mb.readHreg(MODBUS_SLAVE_ID, GREE_HREG_R_OUTDOOR_TEMP, resultHregs, 2);
        while(mb.slave()) { // Check if transaction is active
        mb.task();
        delay(10);
        }
     
        greeCurrentValueOutdoorTempNew = resultHregs[0];
     }
  
 
     if (!mb.slave()) 
     {
        mb.readHreg(MODBUS_SLAVE_ID, GREE_HREG_R_AIR_RETURN_TEMP, resultHregs, 2);
        while(mb.slave()) { // Check if transaction is active
        mb.task();
        delay(10);
        }
        greeCurrentValueAirReturnTempNew = resultHregs[0];
     }
  
  /* if (!mb.slave()) 
   {      
      mb.readHreg(MODBUS_SLAVE_ID, GREE_HREG_RW_TEMP_LOWER_LIMIT_NRJ, resultHregs, 2);
      while(mb.slave()) { // Check if transaction is active
      mb.task();
      delay(10);
      }
      greeCurrentValueTempLowerLimitNrj = resultHregs[0];
   }
   if (!mb.slave()) 
   {      
      mb.readHreg(MODBUS_SLAVE_ID, GREE_HREG_RW_TEMP_UPPER_LIMIT_NRJ, resultHregs, 2);
      while(mb.slave()) { // Check if transaction is active
      mb.task();
      delay(10);
      }
      greeCurrentValueTempUpperLimitNrj = resultHregs[0];
   }*/

      debugPrintln(String("GREE POWER        : " + String(greeCurrentValuePowerNew)).c_str());
      debugPrintln(String("GREE AMBIANT TEMP : " + String(greeCurrentValueAmbiantTempNew)).c_str());
      debugPrintln(String("GREE MODE         : " + String(greeCurrentValueModeNew)).c_str());
      debugPrintln(String("GREE FAN SPEED    : " + String(greeCurrentValueFanSpeedNew)).c_str());   
      debugPrintln(String("GREE TEMPERATURE  : " + String(greeCurrentValueTemperatureNew)).c_str());
    //  debugPrintln(String("GREE TEMP LOWER LIMIT NRJ  : " + String(greeCurrentValueTempLowerLimitNrj)).c_str());      
    //  debugPrintln(String("GREE TEMP UPPER LIMIT NRJ  : " + String(greeCurrentValueTempUpperLimitNrj)).c_str()); 
      debugPrintln(String("GREE SLEEP MODE  : " + String(greeCurrentValueSleepModeNew)).c_str());   
      debugPrintln(String("GREE OUTDOOR TEMP  : " + String(greeCurrentValueOutdoorTempNew)).c_str());
      debugPrintln(String("GREE AIR RETURN TEMP  : " + String(greeCurrentValueAirReturnTempNew)).c_str());
      
   
    
}
void sendModbusCoreValues()
{
      if(greeCurrentValuePowerNew != greeCurrentValuePower)
      {
        greeCurrentValuePower = greeCurrentValuePowerNew;
        enqueueMqttPublish("GREE/corestatus/power", String(greeCurrentValuePower).c_str());
      }
      if(greeCurrentValueAmbiantTempNew != greeCurrentValueAmbiantTemp)
      {
        greeCurrentValueAmbiantTemp = greeCurrentValueAmbiantTempNew;      
        enqueueMqttPublish("GREE/corestatus/ambianttemp", String(greeCurrentValueAmbiantTemp).c_str());
      }
      if(greeCurrentValueModeNew != greeCurrentValueMode)
      {
        greeCurrentValueMode = greeCurrentValueModeNew;      
        enqueueMqttPublish("GREE/corestatus/mode", String(greeCurrentValueMode).c_str());
      }
      if(greeCurrentValueFanSpeedNew != greeCurrentValueFanSpeed)
      {
        greeCurrentValueFanSpeed = greeCurrentValueFanSpeedNew;      
        enqueueMqttPublish("GREE/corestatus/fanspeed", String(greeCurrentValueFanSpeed).c_str());
      }
      if(greeCurrentValueTemperatureNew != greeCurrentValueTemperature)
      {
        greeCurrentValueTemperature = greeCurrentValueTemperatureNew;      
        enqueueMqttPublish("GREE/corestatus/temperature", String(greeCurrentValueTemperature).c_str());
      }
    //  myMqtt->publishValue("GREE/corestatus/templowerlimitnrj", String(greeCurrentValueTempLowerLimitNrj).c_str());
    //  myMqtt->publishValue("GREE/corestatus/tempupperlimitnrj", String(greeCurrentValueTempUpperLimitNrj).c_str());
      if(greeCurrentValueSleepModeNew != greeCurrentValueSleepMode)
      {
        greeCurrentValueSleepMode = greeCurrentValueSleepModeNew;      
        enqueueMqttPublish("GREE/corestatus/sleepmode", String(greeCurrentValueSleepMode).c_str());
      }
      if(greeCurrentValueOutdoorTempNew != greeCurrentValueOutdoorTemp)
      {
        greeCurrentValueOutdoorTemp = greeCurrentValueOutdoorTempNew;      
        enqueueMqttPublish("GREE/corestatus/outdoortemp", String(greeCurrentValueOutdoorTemp).c_str());
      }
      if(greeCurrentValueAirReturnTempNew != greeCurrentValueAirReturnTemp)
      {
        greeCurrentValueAirReturnTemp = greeCurrentValueAirReturnTempNew;      
        enqueueMqttPublish("GREE/corestatus/airreturntemp", String(greeCurrentValueAirReturnTemp).c_str());
      }
 
}

void greeWriteHreg(int reg, int value)
{
   uint16_t tempValue = value;
   if (!mb.slave()) 
   {
      mb.writeHreg(MODBUS_SLAVE_ID, reg, &tempValue, 1);
      while(mb.slave()) { // Check if transaction is active
      mb.task();
      delay(10);
    }
   } 
}

void greeWriteCoil(int coil, bool value)
{
   bool tempValue = value;
   if (!mb.slave()) 
   {
      mb.writeCoil(MODBUS_SLAVE_ID, coil, &tempValue, 1);
      while(mb.slave()) { // Check if transaction is active
      mb.task();
      delay(10);
    }
   } 
}

void greeSetPower(bool value)
{
    if(value == true)
    {
        greeWriteHreg(GREE_HREG_RW_ONOFF, GREE_ON);

    }  
    else
    {
        greeWriteHreg(GREE_HREG_RW_ONOFF, GREE_OFF);      
    }
}
void greeSetMode(int greeMode)
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
