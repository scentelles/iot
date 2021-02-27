freemem_in_gb () { 
    read -r _ freemem _ <<< "$(grep --fixed-strings 'MemAvailable' /proc/meminfo)"
    bc <<< "scale=0;${freemem}/1024"
    
}

./startup.sh &

while true
do
  value=$(freemem_in_gb)
  echo $value
  if (($value < 170))
  then
    echo "not enough memory. Restarting chromium"
    pkill -f chromium
    ./startup.sh &
  fi
  sleep 2
done
