/*
  ModbusRTU ESP8266/ESP32
  Simple slave example

  (c)2019 Alexander Emelianov (a.m.emelianov@gmail.com)
  https://github.com/emelianov/modbus-esp8266

  modified 13 May 2020
  by brainelectronics

  This code is licensed under the BSD New License. See LICENSE.txt for more info.
*/

#include <ModbusRTU.h>

#define REGN1 0
#define REGN2 10
#define REGN3 11
#define SLAVE_ID 1

#define UART_RX 16
#define UART_TX 17
#define UART_RXTX 5
ModbusRTU mb;

//================== HREGS ====================

#define NB_HREGS 84
#define NB_COILS 216

String coils[NB_COILS];
String hregs[NB_HREGS];

#define GREE_HREG_RW_ONOFF 2
#define GREE_ON 0xAA
#define GREE_OFF 0x55

#define GREE_HREG_R_AMBIANT_TEMP       4
#define GREE_HREG_R_IDU_ADDRESS        5

#define GREE_HREG_RW_SET_MODE_ADDRESS   17
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
#define GREE_HREG_RW_SLEEP_MODE            36
#define GREE_HREG_R_AMBIANT_TEMP_SELECT   39
#define GREE_AMBIANT_AIR_RETURN    1
#define GREE_AMBIANT_WIRED_CONTROLLER  2
#define GREE_AMBIANT_MIXED         3

#define GREE_HREG_R_OUTDOOR_TEMP        49
#define GREE_HREG_R_DRED                77
#define GREE_HREG_R_AIR_RETURN_TEMP     82
#define GREE_HREG_R_LIGHT_BOARD_TEMP    83




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
#define GREE_ERROR_R_JUMPER_CAP                         165
#define GREE_ERROR_R_ODU_MEMORY                         166
#define GREE_ERROR_R_COM_DRIVE                          174
#define GREE_STATUS_R_SAVE                              176
#define GREE_STATUS_R_COOL_ONLY_HEAT_PUMP_FLAG          177
#define GREE_STATUS_R_SYSTEM_DEFROSTING                 179
#define GREE_STATUS_R_LOW_POWER_CONSUMPTION             182
#define GREE_ERROR_R_AC_PHASE                           183
//goes up to 215

void setup() {
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, UART_RX, UART_TX);

  mb.begin(&Serial2, UART_RXTX);

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
  hregs[36] = "GREE_HREG_RW_SLEEP_MODE";
  hregs[39] = "GREE_HREG_R_AMBIANT_TEMP_SELECT";
  hregs[49] = "GREE_HREG_R_OUTDOOR_TEMP";
  hregs[77] = "GREE_HREG_R_DRED";
  hregs[82] = "GREE_HREG_R_AIR_RETURN_TEMP";
  hregs[83] = "GREE_HREG_R_LIGHT_BOARD_TEMP";


  for(int i = 0; i < NB_COILS; i++)
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


  mb.slave(SLAVE_ID);
  mb.addHreg(REGN1);
  mb.Hreg(REGN1, 7);
  mb.addHreg(REGN2, 8);
  mb.Hreg(REGN2, 9);
  mb.addHreg(REGN3);
  mb.Hreg(REGN3, 5);

  for (int i = 0; i < NB_HREGS; i++)
  {
    if(String(hregs[i]) != "RESERVED")
    {
      Serial.println("adding hreg");
      mb.addHreg(i);
      mb.Hreg(i, i+10);
    }
  }

  for (int i = 0; i < NB_COILS; i++)
  {
    if(String(coils[i]) != "RESERVED")
    {
      Serial.println("adding coil");
      mb.addCoil(i);
      mb.Coil(i, true);
    }
  }
}

void loop() {
  
  mb.task();
  yield();

 /* while(Serial2.available())
  {
    Serial.println(Serial2.read());
  }*/

  
}
