    
echo "startup door script"
sudo python3 -u /home/iot/Projects/iot/server/door-proxmox.py > /home/iot/Projects/door_log.txt &
sleep 1

echo "startup system mqtt script"  
python3 -u /home/iot/Projects/iot/server/system_mqtt.py > /home/iot/Projects/system_log.txt &

sleep 1   

echo "startup RF script"
python3 -u /home/iot/Projects/iot/server/RFbridge.py > /home/iot/Projects/RFbridge.log &

echo "startup AIRC"
python3 -u /home/iot/Projects/iot/server/AirC/airc_control.py > /home/iot/Projects/AirC.log &
