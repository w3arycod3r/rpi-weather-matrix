#!/usr/bin/env python3
# /\ Tells the UNIX shell which python interpreter to use...need UNIX EOL's on this file

# Title: get_verse.py
# Author: Garrett Carter
# Date: 6/21/19
# Purpose: Use the ESV Bible API to fetch the verse-of-the-day and store it

import sys
from os import kill
import signal
import json
import requests
import time


#======# CONSTANTS & DIRECTORIES
API_KEY = 'XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX'
API_URL = 'https://api.esv.org/v3/passage/text/'

SHARE_DIR = '../out/' # Shared output files
PYTHON_DEV = './' # Current directory
#SHARE_DIR = ''
#PYTHON_DEV = ''

VERSE_FILE = SHARE_DIR + 'verse.txt'
LOG_FILE = SHARE_DIR + 'verse_log.txt'
VOTD_DATABASE = PYTHON_DEV + 'votd.json'
PID_FILE = SHARE_DIR + 'weather_pid.txt'


#======# FUNCTIONS
def exit_with_error(s):
    print('Error: %s' % s)

    exit()


def get_esv_text(passage):
    params = {
        'q': passage,
        #'include-short-copyright': False,
        'include-verse-numbers': False,
        'include-first-verse-numbers': False,
        'include-footnotes': False,
        'include-headings': False,
        'indent-paragraphs': 0,
        'indent-poetry': False,
        'indent-declares': 0,
        'indent-psalm-doxology': 0,
    }

    headers = {
        'Authorization': 'Token %s' % API_KEY
    }

    response = requests.get(API_URL, params=params, headers=headers)

    text = response.json()['passages'][0]
    # Format Text for matrix ticker
    text = text.replace('\n',' /',1)
    text = text.replace('\n','/ ',1)
    text = text.replace('\n',' ')
    # Replace en-dash with hyphen (only a hyphen is present in our font)
    text = text.replace('–','-')
    # Replace em-dash
    text = text.replace('—','-')
    # Remove trailing comma
    text = text.replace(', (ESV)',' (ESV)')

    return text


def render_reading(passage):
    """Pass a passage ref string in a format the ESV API will like"""

    text = get_esv_text(passage)

    print(text)
    #print(''.join(texts)) # Texts is an array of strings

def write_reading(passage):
    text = get_esv_text(passage)

    file_obj = open(VERSE_FILE, 'w')
    file_obj.write(text)
    file_obj.close()


def write_log(msg):
    time_string = time.asctime()

    file_obj = open(LOG_FILE, 'a')
    file_obj.write(msg + ' @ ' + time_string + '\n')
    file_obj.close()


def is_leap_year(year):
    """Determine whether a year is a leap year."""

    return year % 4 == 0 and (year % 100 != 0 or year % 400 == 0)

def get_today_number():
    st = time.localtime()

    if is_leap_year(st.tm_year):
        return st.tm_yday
    else:
        if st.tm_mon >= 3: # Past Feb. 29 position
            return st.tm_yday + 1
        else:
            return st.tm_yday

def send_signal():
    """Sends SIGRTMIN to the PID given in PID_FILE"""
    file_obj = open(PID_FILE, 'r')
    pid = int(file_obj.read())
    file_obj.close()

    try:
        kill(pid, signal.SIGRTMIN)
    except ProcessLookupError:
        write_log('Invalid PID')
        exit_with_error('Invalid PID')


#======# MAIN FUNCTION
if __name__ == '__main__':
    if len(sys.argv) < 2:
        day_number = get_today_number()
    
    else:
        day_number = sys.argv[1]
        if not day_number.isdigit():
            exit_with_error('Invalid day number')
        day_number = int(day_number)


    entries = json.load(open(VOTD_DATABASE))['verses']

    if day_number < 1 or day_number > len(entries):
        exit_with_error('Day number out of range')

    render_reading(entries[day_number-1])

    write_reading(entries[day_number-1])

    write_log('Day ' + str(day_number) +  ' // verse write success')

    send_signal()