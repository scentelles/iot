#!/bin/bash
 
# Vérifie que l'on est en Root
if [[ $EUID -ne 0 ]];
then
    echo "Ce script doit être exécuté avec les privilèges administrateur"
    exit 1
fi
 



hciconfig hci0 down
hciconfig hci0 up


get_handler()
{ 
echo "Getting info from $MACadd"
hnd38=$(timeout 20 gatttool -b $MACadd --char-write-req --handle='0x0038' --value="0100" --listen | grep --max-count=1 "Notification handle")
# Notification handle = 0x0036 value: 7c 08 2a 97 0c
}

get_values_from_sensor()
{ 
if !([ -z "$hnd38" ])
then
 
    temperature=${hnd38:39:2}${hnd38:36:2}
    temperature=$((16#$temperature))
    if [ "$temperature" -gt "10000" ];
    then
        temperature=$((-65536 + $temperature))
    fi 
    temperature=$(echo "scale=2;$temperature/100" | bc)
 
    humidity=${hnd38:42:2}
    humidity=$((16#$humidity))
 
    voltage=${hnd38:48:2}${hnd38:45:2}
    voltage=$((16#$voltage))
    voltage=$(echo "scale=3;$voltage/1000" | bc)
 
 
    # Battery Level : 0x2A19
    # handle = 0x001b, uuid = 00002a19-0000-1000-8000-00805f9b34fb
 
        hnd1b=$(gatttool --device=$MACadd --char-read -a 0x1b)
        # Characteristic value/descriptor: 63
        battery=${hnd1b:33:2}
        battery=$((16#$battery))
 
    # Firmware Revision String : 0x2A26
    # handle = 0x0012, uuid = 00002a26-0000-1000-8000-00805f9b34fb
 
        hnd12=$(gatttool --device=$MACadd --char-read -a 0x12)
        # Characteristic value/descriptor: 31 2e 30 2e 30 5f 30 31 30 36 00
        firmware=$(echo "'$hnd12'" | cut -c34-65 | xxd -r -p)
 
 
    # Device Name : 0x2A00
    # handle = 0x0003, uuid = 00002a00-0000-1000-8000-00805f9b34fb
 
        hnd03=$(gatttool --device=$MACadd --char-read -a 0x03)
        # Characteristic value/descriptor: 4c 59 57 53 44 30 33 4d 4d 43 00
        name=$(echo "'$hnd03'" | cut -c34-65 | xxd -r -p)
 
 
    # Hardware Revision String : 0x2A27
    # handle = 0x0014, uuid = 00002a27-0000-1000-8000-00805f9b34fb
 
        hnd14=$(gatttool --device=$MACadd --char-read -a 0x14)
        # Characteristic value/descriptor: 42 31 2e 34
        revision=$(echo "'$hnd14'" | cut -c34-45 | xxd -r -p)
 
    # Affichage des données
 
        echo "Reading Mijia..."
        echo "Hardware: {Firmware Version:$firmware / Hardware Revision:$revision / BT Name:$name / Battery Level:$battery}"
        echo "Sensors: {Temperature:$temperature / Humidity:$humidity / Voltage:$voltage}"
        echo "publishing value on $MQTT_path"
        mosquitto_pub -h $MQTT_ADDRESS -t $MQTT_path/temp -m $temperature
        mosquitto_pub -h $MQTT_ADDRESS -t $MQTT_path/hum -m $humidity
        mosquitto_pub -h $MQTT_ADDRESS -t $MQTT_path/bat -m $battery

else
 
    echo "Mijia error"
 
fi
}

MQTT_ADDRESS="192.168.1.27"
while true
do
  MACadd="A4:C1:38:F9:50:17"
  MQTT_path="mijia1"
  get_handler
  get_values_from_sensor
  sleep 5
  MACadd="A4:C1:38:89:3A:0C"
  MQTT_path="mijia2"
  get_handler
  get_values_from_sensor
  sleep 5
  MACadd="A4:C1:38:50:4B:C7"
  MQTT_path="mijia3"
  get_handler
  get_values_from_sensor
  sleep 5
  MACadd="A4:C1:38:91:E7:1F"
  MQTT_path="mijia4"
  get_handler
  get_values_from_sensor
  
  sleep 30
done
