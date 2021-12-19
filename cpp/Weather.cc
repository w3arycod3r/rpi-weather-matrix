/*
	Title: Weather.cpp
	Author: Garrett Carter
	Date: 6/27/19
	Purpose: Define Weather class functions
*/

#include "Weather.h"
#include <fstream>
#include <iostream>

void WeatherData::init()
{
	// Current
	currSummary = "";
	iconMap = 0;
	temp = 0;
	apparentTemp = 0;

	// Week Ahead
	weekSummary = "";

	// Today
	todaySummary = "";
	sunrise = 0;
	sunset = 0;
	moonPhaseIcon = 0;
	precipProb = 0; // Out of 100
	precipType = "";
	high = 0;
	low = 0;

	// New Stuff
	humidity = 0;
	uvIndex = 0;
	cloudCover = 0;
	windGust = 0;
	windBearing = 0;
	windDir = "";
	visibility = 0;
	ozone = 0;
	pressure = 0;
	moonPhase = 0;
	dewPoint = 0;
	lastUpdated = 0;

}

WeatherData::WeatherData()
{
	init();
}

WeatherData::WeatherData(const string filePath)
{
	// If reading fails, init everything
	if (this->readFromFile(filePath) == false)
		init();
}

WeatherData::~WeatherData()
{
	
}

bool WeatherData::readFromFile(const string filePath)
{
	bool fileGood = true;
	ifstream fd(filePath.c_str()); // Open for reading
	
	if (!fd.good()) // file doesn't exist
		fileGood = false;
	
	if (fd.good())
	{
		vector<string> lines;
		string buff;

		while(fd.good()) // Write each line into string vector
		{
			getline(fd,buff);
			lines.push_back(buff);
		}

		// Assign values to attributes
		currSummary = 	lines[0];
		iconMap = 		stoi(lines[1]);
		temp = 			stoi(lines[2]);
		apparentTemp =  stoi(lines[3]);

		weekSummary = 	lines[4];

		todaySummary = 	lines[5];

		// Read Sunrise and Sunset UNIX timestamps
		sunrise = 		stoi(lines[6]);
		sunset = 		stoi(lines[7]);


		moonPhaseIcon = stoi(lines[8]);

		// Precip
		precipProb = 	stoi(lines[9]);

		precipType = 	lines[10];
		
		// High and Low Temps
		high =			stoi(lines[11]);
		low = 			stoi(lines[12]);

		// New Stuff
		humidity =		stoi(lines[13]);
		uvIndex =		stoi(lines[14]);
		cloudCover = 	stoi(lines[15]);
		windGust = 		stof(lines[16]);
		windBearing =	stoi(lines[17]);
		windDir = 			 lines[18];
		visibility = 	stof(lines[19]);
		ozone = 		stof(lines[20]);
		pressure =		stof(lines[21]);
		moonPhase =		stoi(lines[22]);
		dewPoint = 		stof(lines[23]);
		lastUpdated = 	stoi(lines[24]);

	}
	
	return fileGood;
}



void WeatherData::printDebugData() const
{
	cerr << "\nWeather Data:\n\n";
	cerr << currSummary << endl;
	cerr << iconMap << endl;
	cerr << temp << endl;
	cerr << apparentTemp << endl;

	// Week Ahead
	cerr << weekSummary << endl;

	// Today
	cerr << todaySummary << endl;

	cerr << sunrise << endl;
	cerr << sunset << endl;
	cerr << moonPhaseIcon << endl;
	cerr << precipProb << endl; // Out of 100
	cerr << precipType << endl;
	cerr << high << endl;
	cerr << low << endl;

	// New Stuff
	cerr << humidity 	<< endl;
	cerr << uvIndex 	<< endl;
	cerr << cloudCover 	<< endl;
	cerr << windGust 	<< endl;
	cerr << windBearing << endl;
	cerr << windDir		<< endl;
	cerr << visibility 	<< endl;

	cerr << ozone 		<< endl;
	cerr << pressure 	<< endl;
	cerr << moonPhase 	<< endl;
	cerr << dewPoint 	<< endl;
	cerr << lastUpdated << endl;
}