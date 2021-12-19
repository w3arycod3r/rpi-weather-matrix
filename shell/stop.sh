#!/bin/bash
# Name:    stop.sh
# Purpose: Shell script to politely stop the weather display daemon.
#          Can be ran from any directory, by root user or pi user.
SERVICE="weather-disp"

if sudo killall "$SERVICE"
then
    echo "$SERVICE daemon was sent SIGTERM"
else
    echo "$SERVICE daemon was not running"
fi
