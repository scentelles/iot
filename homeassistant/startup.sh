#sudo -u homeassistant -H -s
#source /srv/homeassistant/bin/activate

unclutter &

chromium http://localhost:8123 --kiosk --disk-cache-dir=/dev/null --disk-cache-size=1 --media-cache-size=1

#vcgencmd display_power 1
