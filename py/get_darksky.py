#!/usr/bin/env python3
# /\ Tells the UNIX shell which python interpreter to use...need UNIX EOL's on this file

# Title: get_darksky.py
# Author: Garrett Carter
# Date: 6/25/19
# Purpose: Use the DarkSky API to fetch weather data and store it

import requests, json, time, sys, signal
from os import kill


#=====# CONSTANTS
API_KEY = 'XXXXXXXXXXXXXXXXXXXXX'
LOCATION_COORDS = 'XX.XXXX, XX.XXXX' # Latitude,Longitude
API_URL = 'https://api.darksky.net/forecast/' + API_KEY + '/' + LOCATION_COORDS


#=====# FILES
SHARE_DIR = '../out/' # Shared output files
RAW_JSON = SHARE_DIR + 'raw_weather.json'
DATA_FILE = SHARE_DIR + 'weather_data.txt'

LOG_FILE = SHARE_DIR + 'weather_log.txt'
PID_FILE = SHARE_DIR + 'weather_pid.txt'


#=====# FUNCTIONS
def get_weather_response():
    """Returns the response object from the API call"""
    params = {
        'exclude': 'minutely,hourly,flags',
        'Accept-Encoding': 'gzip',
    }

    response = requests.get(API_URL, params=params)

    write_log('Pulled from DarkSky', 0)

    return response


def write_weather():
    """Processes and writes selected weather data into DATA_FILE"""
    r = get_weather_response() # Automatically decodes gzip :D
    j = r.json()

    write_raw_data(r)

    curr = j['currently']
    daily = j['daily']
    today = daily['data'][0]

    #print(j)
    data = []



    #====# Extract Data
    try:
        data.append(curr['summary'])
    except KeyError:
        write_log('curr[summary] not defined', 1)
        data.append('Error')

    try:
        data.append(decode_icon(curr['icon'])) # Decode icon string and write an integer to the file
    except KeyError:
        write_log('icon not defined', 1)
        data.append(0)

    try:
        data.append(round(curr['temperature']))
    except KeyError:
        write_log('temperature not defined', 1)
        data.append(0)

    try:
        data.append(round(curr['apparentTemperature']))
    except KeyError:
        write_log('apparentTemperature not defined', 1)
        data.append(0)

    try:
        data.append(daily['summary']) # Week ahead
    except KeyError:
        write_log('daily[summary] not defined', 1)
        data.append('Error')

    try:
        data.append(today['summary'])
    except KeyError:
        write_log('today[summary] not defined', 1)
        data.append('Error')

    try:
        data.append(today['sunriseTime'])
    except KeyError:
        write_log('sunriseTime not defined', 1)
        data.append(0)

    try:
        data.append(today['sunsetTime'])
    except KeyError:
        write_log('sunsetTime not defined', 1)
        data.append(0)

    try:
        data.append(decode_moon(today['moonPhase'])) # Decode lunation number and write an integer to the file (corresponds to an icon)
    except KeyError:
        write_log('moonPhase not defined', 1)
        data.append(0)

    try:
        data.append(round(today['precipProbability'] * 100)) # Dec to %
    except KeyError:
        write_log('precipProbability not defined', 1)
        data.append(101)

    try:
        data.append(today['precipType'])
    except KeyError:
        write_log('precipType not defined', 1)
        data.append(' ')

    try:
        data.append(round(today['temperatureHigh']))
    except KeyError:
        write_log('temperatureHigh not defined', 1)
        data.append(0)

    try:
        data.append(round(today['temperatureLow']))
    except KeyError:
        write_log('temperatureLow not defined', 1)
        data.append(0)

    # New Stuff
    try:
        data.append(round(curr['humidity'] * 100)) # % rel humidity
    except KeyError:
        write_log('humidity not defined', 1)
        data.append(0)

    try:
        data.append(round(today['uvIndex']))
    except KeyError:
        write_log('uvIndex not defined', 1)
        data.append(0)

    try:
        data.append(round(curr['cloudCover'] * 100)) # % cloud coverage
    except KeyError:
        write_log('cloudCover not defined', 1)
        data.append(0)

    try:
        data.append(round(curr['windGust'], 1))
    except KeyError:
        write_log('windGust not defined', 1)
        data.append(0)

    try:
        deg = int(curr['windBearing'])
        data.append(deg)
        data.append(decode_wind(deg))
    except KeyError:
        write_log('windBearing not defined', 1)
        data.append(0)
        data.append(' ')

    try:
        data.append(round(curr['visibility'], 1)) # Round to one decimal place X.x (Caps at 10 mi)
    except KeyError:
        write_log('visibility not defined', 1)
        data.append(0)

    try:
        data.append(round(curr['ozone'], 1))
    except KeyError:
        write_log('ozone not defined', 1)
        data.append(0)

    try:
        data.append(round(curr['pressure'], 2))
    except KeyError:
        write_log('pressure not defined', 1)
        data.append(0)

    try:
        data.append(round(today['moonPhase'] * 100)) # % of moon phase cycle
    except KeyError:
        write_log('moonPhase not defined', 1)
        data.append(0)

    try:
        data.append(round(curr['dewPoint'], 2))
    except KeyError:
        write_log('dewPoint not defined', 1)
        data.append(0)
    
    try:
        data.append(round(curr['time']))
    except KeyError:
        write_log('time not defined', 1)
        data.append(0)


    #====# Write Data
    file_obj = open(DATA_FILE,'w')

    for var in data:
        file_obj.write(str(var) + '\n')
    file_obj.close()

    write_log('Processed data written into ' + DATA_FILE, 0)


def decode_icon(icon_str):
    '''Returns an integer icon value decoded from the string given by DS API'''
    default_val = 0   # Default icon in case of no matching element (Celsius Icon)

    icon_map = {
         'clear-day'             : 20
        ,'clear-night'           : 17
        ,'rain'                  : 13
        ,'snow'                  : 15
        ,'sleet'                 : 8
        ,'wind'                  : 29
        ,'fog'                   : 1
        ,'cloudy'                : 4
        ,'partly-cloudy-day'     : 3
        ,'partly-cloudy-night'   : 5
    }

    return icon_map.get(icon_str, default_val)


def decode_moon(lun_num):
    '''Returns an integer icon value decoded from the fractional lunation number given by DS API'''
    default_val = 0   # Default icon in case of no matching element

    moon_map = {
         0  : 30    # New Moon
        ,1  : 31    # Waxing Crescent
        ,2  : 32    # First Quarter
        ,3  : 33    # Waxing Gibbous
        ,4  : 34    # Full Moon
        ,5  : 35    # Waning Gibbous
        ,6  : 36    # Last Quarter
        ,7  : 37    # Waning Crescent
        ,8  : 30    # New Moon
    }

    return moon_map.get(round(lun_num*8), default_val)

def decode_wind(deg):
    '''Returns a direction string decoded from the degrees CW from north given by DS API'''
    default_val = 'N'   # Default in case of no matching element

    wind_map = {
         0  : 'N'
        ,1  : 'NE'
        ,2  : 'E'
        ,3  : 'SE'
        ,4  : 'S'
        ,5  : 'SW'
        ,6  : 'W'
        ,7  : 'NW'
        ,8  : 'N'
    }

    return wind_map.get(round(deg/45.0), default_val)

def write_raw_data(r):
    '''Writes complete text data of response object r into RAW_JSON'''
    text = r.text

    file_obj = open(RAW_JSON,'w')
    file_obj.write(text)
    file_obj.close()

    write_log('Raw data written into ' + RAW_JSON, 0)


def exit_with_error(s):
    print_error(s)

    exit()

def print_error(s):
    print('Error: %s' % s)

def print_msg(msg):
    print('Msg: %s' % msg)


def write_log(msg, whatType):
    '''Writes the string msg and a time stamp into LOG_FILE
       whatType: 0 = info message, 1 = error'''

    time_string = time.asctime()
    process_name = 'get_darksky.py'
    log_format = '{:24s} - {:16s} - {:5s} : '

    file_obj = open(LOG_FILE, 'a')
    # Info Message
    if whatType == 0:
        print_msg(msg)

        log_string = log_format.format(time_string, process_name, 'Msg') + msg + '\n'

        file_obj.write(log_string)

    # Error
    else:
        print_error(msg)

        log_string = log_format.format(time_string, process_name, 'Error') + msg + '\n'
        
        file_obj.write(log_string)
    
    file_obj.close()


def send_signal():
    """Sends SIGRTMIN+1 to the PID given in PID_FILE"""
    try:
        sigVal = signal.SIGRTMIN+1
        file_obj = open(PID_FILE, 'r')
        pid = int(file_obj.read())
        file_obj.close()

        kill(pid, sigVal)

        write_log('Signal {0} sent to PID {1}'.format(sigVal, pid), 0)

    except ProcessLookupError:
        write_log('Invalid PID', 1)
    except:
        write_log('Signal could not be sent', 1)

#=====# MAIN FUNCTION

if __name__ == '__main__':
    write_weather()

    send_signal()
