/*
	Title: Weather.h
	Author: Garrett Carter
	Date: 6/27/19
	Purpose: Provide misc functions and classes for weather program
*/

#ifndef WEATHER_H
#define WEATHER_H

#include <ctime>
#include <string>
#include <iostream>
#include <vector>

using std::string; using std::ifstream; using std::cerr; using std::endl;
using std::stof; using std::vector;

class WeatherData
{

private:
	// init(): initialize all members to default or empty values
	void init();

		
public:
	//=====// Attributes

	// Current
	string currSummary;
	int temp, apparentTemp;

	/*
		iconMap: A mapping from icon string to actual icon files to be displayed
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
    }
	 */
	int iconMap;

	// Week Ahead
	string weekSummary;

	// Today
	string todaySummary;
	time_t sunrise;
	time_t sunset;

	/*
		moonPhaseIcon: value returned from below python map

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
	*/
	int moonPhaseIcon;

	int precipProb; // Out of 100
	string precipType;
	int high;
	int low;
	
	// New Stuff
	int humidity; // % rel humidity
	int uvIndex;
	int cloudCover; // % cloud coverage
	float windGust; // wind gust speed in mph
	int windBearing; // wind coming from x degrees CW of true north
	string windDir; // wind direction in readable format
	float visibility; // visibility in miles
	float ozone; // Ozone conc. in DU
	float pressure; // Atmospheric pressure in mb
	int moonPhase; // Moon phase cycle in %
	float dewPoint; // Dew point in deg Fahrenheit
	time_t lastUpdated; // Timestamp of last API call

	//=====// Functions

	// WeatherData(): calls init()
	WeatherData();

	// WeatherData(filePath): Trys calling readFromFile on filePath, calls init() upon failure. Vars will be initialized.
	WeatherData(const string filePath);
	
	// Empty
	~WeatherData();

	// readFromFile(): Attempts to fill attributes from file. Upon failure, returns false and nothing is done.
	bool readFromFile(const string filePath);
	
	// printDebugData(): Prints each attribute for debugging purposes
	void printDebugData() const;
		
};

#endif