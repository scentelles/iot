#launch this script at startup from /etc/rc.local 

echo "startup script start"
sleep 10
    
echo "startup door script"
python3 -u /home/kubuntu/Projects/iot/server/door-proxmox.py > /home/kubuntu/Projects/door_log.txt &
sleep 1

echo "startup system mqtt script"  
python3 -u /home/kubuntu/Projects/iot/server/system_mqtt.py > /home/kubuntu/Projects/system_log.txt &
sleep 1   

cd /home/kubuntu/Projects/iot/server
 
echo "startup RF script"
python3 -u /home/kubuntu/Projects/iot/server/RFbridge.py > /home/kubuntu/Projects/RFbridge.log &
sleep 1

echo "startup AirC script"
python3 -u /home/kubuntu/Projects/iot/server/AirC/airc_control.py > /home/kubuntu/Projects/AirC.log &
sleep 1

    
#Launch VNC virtual desktop
#echo "Launching virtual desktop"
#vncserver -geometry 1920x1080 :2 
    
#sleep 10
#xscreensaver-command -display :2.0 -exit
    
#    echo "Launching motioneye"    
#    sudo systemctl start motioneye
    
#    echo "startup script stop \n" >> /home/pi/startup.log


