#sudo -u homeassistant -H -s
#source /srv/homeassistant/bin/activate

unclutter &

chromium-browser http://localhost:8123 --window-size=1920,1080 --start-fullscreen --kiosk

#vcgencmd display_power 1
