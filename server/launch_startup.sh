

if pgrep ECS_scheduler
then
    echo "first session already running\n" >> /home/pi/startup.log
    exit
else
    echo "startup script start \n" >> /home/pi/startup.log
    echo "startup script start"
    sleep 10
    
    echo "startup door script"
    python3 -u /home/pi/Projects/iot/server/door.py >> /home/pi/Projects/door_log.txt &
    sleep 1

    echo "startup system mqtt script"  
    python3 -u /home/pi/Projects/iot/server/system_mqtt.py >> /home/pi/Projects/system_log.txt &

    sleep 1   

    cd /home/pi/Projects/iot/server
 
    echo "startup ECS script"
    /home/pi/Projects/iot/server/ECS_scheduler_new.py >> /home/pi/Projects/ECS_log.txt &
    sleep 1

    echo "startup RF script"
    python3 -u /home/pi/Projects/iot/server/RFbridge.py >> /home/pi/Projects/RFbridge.log &
    sleep 1

#    echo "startup TTS script"
#    python3 -u /home/pi/Projects/iot/server/TTS_daemon.py >> /home/pi/Projects/TTS_daemon.log &
#    sleep 1

    echo "startup TTS script"
    python3 -u /home/pi/Projects/iot/server/AirC/airc_control.py >> /home/pi/Projects/AirC.log &
    sleep 1

    
#    /usr/bin/node-red start >> /home/pi/Projects/node-red.log &
    
    echo "launching node-red manually to be after usb drive mount"
    /usr/bin/influxd &
    
    #python -u /home/pi/Projects/camera.py >> /home/pi/Projects/camera_log.txt &
    echo "launching node-red"
    /usr/bin/node-red /home/pi/Projects/iot/node-red/myflow.json >> /home/pi/Projects/node-red.log &
   
    #echo "mounting samba SHIELD share" 
    #sudo mount -t cifs -o username=scentelles,password=tweak-were-bingle,uid=1000,gid=1000 //192.168.1.9/Elements/NVIDIA_SHIELD/Films/tmp /home/pi/Shield
    
    #echo "mounting samba Synology  share" 
    #sudo mount -t cifs -o username=admin,password=Totor0608,uid=1000,gid=1000 //192.168.1.1/usbshare1/Download /home/pi/Synology

    
    #Launch VNC virtual desktop
    echo "Launching virtual desktop"
    vncserver -geometry 1920x1080 :2 
    
    sleep 10
    xscreensaver-command -display :2.0 -exit
    
    echo "Launching motioneye"    
    sudo systemctl start motioneye
    
    echo "startup script stop \n" >> /home/pi/startup.log

fi
