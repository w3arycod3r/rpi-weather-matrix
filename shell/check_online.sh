#!/bin/bash
# Name:    check_online.sh
# Purpose: Gives some feedback to an ssh command to check if pi is up and running.
SERVICE="weather-disp"

echo "$HOSTNAME is alive and online"

if pgrep -x "$SERVICE" >/dev/null
then
    echo "$SERVICE daemon IS running"
else
    echo "$SERVICE daemon IS NOT running"
fi
