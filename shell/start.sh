#!/bin/bash
# Name:    start.sh
# Purpose: Shell script to conditionally start the weather display program as a daemon.
#          Can be ran from any directory, by root user or pi user.
SERVICE="weather-disp"
FILEPATH="/home/pi/matrix/c++/"

if pgrep -x "$SERVICE" >/dev/null
then
    echo "$SERVICE already running"
else
    echo "Starting $SERVICE daemon"
    cd $FILEPATH
    sudo ./$SERVICE -d
fi
