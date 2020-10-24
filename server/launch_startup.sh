

if pgrep Xtightvnc
then
    echo "first session already running\n" >> /home/pi/startup.log
    exit
else
    echo "startup script start \n" >> /home/pi/startup.log
    sleep 10
    
    
    python -u /home/pi/Projects/iot/server/door.py >> /home/pi/Projects/door_log.txt &
    sleep 1

    
    sleep 1
    python -u /home/pi/Projects/temp_mqtt.py >> /home/pi/Projects/temp_log.txt &
    sleep 1
    python -u /home/pi/Projects/mqtt_loop.py >> /home/pi/Projects/mqtt_web_log.txt &
    sleep 1
    python -u /home/pi/Projects/system_mqtt.py >> /home/pi/Projects/system_log.txt &
    sleep 1   
    cd /home/pi/Projects/iot/server
 
    python -u /home/pi/Projects/iot/server/ECS_scheduler_new.py >> /home/pi/Projects/ECS_log.txt &
    sleep 1

    python -u /home/pi/Projects/iot/server/RFbridge.py >> /home/pi/Projects/RFbridge.log &
    sleep 1
 

    
#    python -u /home/pi/Projects/iot/server/domoticz_mqtt.py >> /home/pi/Projects/domoticz_mqtt_log.txt &
#    sleep 1
#    /usr/bin/node-red start >> /home/pi/Projects/node-red.log &
    sleep 5
    
    #python -u /home/pi/Projects/camera.py >> /home/pi/Projects/camera_log.txt &

    #sleep 5
    vncserver :1 &
    

    echo "startup script stop \n" >> /home/pi/startup.log

fi
