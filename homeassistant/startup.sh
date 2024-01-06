#!/bin/bash

#sudo -u homeassistant -H -s
#source /srv/homeassistant/bin/activate

#deactivate unclutter, as it overloads CPU when xscreensaver active
#unclutter -idle 0 &
#xscreensaver &
#chromium http://192.168.1.27:8123 --kiosk --disable-features=MojoIpcz
firefox http://192.168.1.46:8123 -kiosk 
#vcgencmd display_power 1
