# Name:    bldrun-weather.sh
# Purpose: First, kill the daemon if it is running. Then, Build and run main executable for weather display program
#          This can be ran from anywhere without changing directories
#          The c++ program uses relative directory addressing
/home/pi/matrix/shell/force_stop.sh
cd /home/pi/matrix/c++/
if make main; then
        echo
        echo "PROGRAM OUTPUT:"
        sudo ./weather-disp
fi