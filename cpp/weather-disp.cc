/*
	Title: 		weather-disp.cc
	Author: 	Garrett Carter
	Date: 		9/2/19
	Purpose: 	Main file for a RGB led matrix display. Scrollable (via rotary encoder) screens containing weather,
				animations, and a settings menu. See weather_config.h for configurable parameters
*/

#include "weather_config.h"


////*********************************************************************
//// MAIN FUNCTION
////*********************************************************************

int main(int argc, char** argv)
{
	// SIGNAL HANDLERS
	signal(SIGTERM, killHandler);
	signal(SIGINT, killHandler);
	signal(SIGUSR1, InputWaker); // Handle wakeup signal from InputThread
	signal(SIGRTMIN, PyHandler);
	signal(SIGRTMIN+1, PyHandler);

	//=====// INITIALIZATION
	loadFonts();
	// Settings
	setDefaultConfig();
	if (!readConfig())
		cerr << "Error reading config file\n";
	printConfig();
	
	
	// Create Matrix Object
	RGBMatrix::Options defaults;
	RuntimeOptions rtOps;
	
	defaults.rows = M_HEIGHT;
	defaults.cols = M_WIDTH;
	if (!currSett["autoBrightness"])
		defaults.brightness = currSett["brightness"];
	else
		defaults.brightness = defSett["brightness"];
	
	rtOps.drop_privileges = 0; // Don't drop root (for shutdown command later)
	if (argc > 1 && strcmp(argv[1], "-d") == 0) // Daemon flag
	{
		rtOps.daemon = 1; // Daemonize
	}
	
	matrix = rgb_matrix::CreateMatrixFromOptions(defaults, rtOps);
	if (matrix == NULL)
	{
		cerr << "Error creating matrix obj\n";
		return 1;
	}
	
	// Start input thread
	Input = new RotInput(pthread_self(),CLK_PIN, DT_PIN, SW_PIN, matrix, PWR_SW_PIN);
	
	// Write PID to file (after daemonize)
	std::ofstream outFile(PID_FILE.c_str());
	if (outFile.good()) // successful open
	{
		outFile << ::getpid();
		outFile.close();
	}
	
	fprintf(stderr,"PID: %d\n",::getpid());
	
	// Read in WEATHER_FILE and VERSE_FILE (existence checks are done)
	wd->readFromFile(WEATHER_FILE);
	readVerse();
	
	// Buffering Canvas
	offscreen = matrix->CreateFrameCanvas();

	
	//=====// MAIN LOOP
	while(!killSigReceived)
	{
		autoBrightness();
		inputLoop();
		drawLoop();

		updateWeather();
		updateVerse();
		
		if (currSett["screen"] == SHUTDOWN && defSett["screen"] != SHUTDOWN && !screenChange)
			system("sudo shutdown -h now");

		writeConfig();
	}
	
	runWriteConfig = true;
	writeConfig();
	delete Input; // Cancel Input thread
	// Do this after cancelling any threads
	matrix->Clear();
	delete matrix;
	delete wd;

	// Cleanup anims and icons
	delete weatherIcons;
	delete ifaceIcons;
	

	fprintf(stderr,"\nBye! :)\n");
	return 0;
}


////*********************************************************************
//// UTILITY FUNCTIONS
////*********************************************************************

void inputLoop()
{		
	// Locals
	RGBMatrix* m = matrix;
	FrameCanvas* c = offscreen;

	switch (Input->getEvent())
	{
	case DIR_CW:
		if (currSett["screen"]==BRIGHT_CHANGE) // Increase brightness
		{
			int b = currSett["brightness"];
			if (b <= 90 && b >= 10)
				b += 10;
			else if (b < 10) // Fine tuning at lower b levels
				b += 1;
			
			if (!currSett["autoBrightness"])
				m->SetBrightness(b);
			currSett["brightness"] = b;
			refreshScreen = true;
			break;
		}
		if (currSett["screen"]==SETTINGS) // Change settings selection
		{
			if (currSett["selection"] < settingSelections.size()-1)
			{
				currSett["selection"]++;
				refreshScreen = true;
				break;
			}
		}
		
		if (currSett["screen"] > LAST_SCREEN) // Only loop around on main screens
			break;
		if (currSett["screen"] == LAST_SCREEN) // Loop around
		{
			currSett["screen"] = FIRST_SCREEN;
			screenChange = true;
			refreshScreen = true;
			break;
		}
		if (currSett["screen"] < LAST_SCREEN) // Move forward to next sequential screen
		{
			currSett["screen"]++;
			screenChange = true;
			refreshScreen = true;
			break;
		}
	case DIR_CCW:
		if (currSett["screen"]==BRIGHT_CHANGE) // Reduce brightness
		{
			int b = currSett["brightness"];
			if (b >= 20)
				b -= 10;
			else if (b <= 10 && b > 1) // Fine tuning at lower b levels
				b -= 1;

			if (!currSett["autoBrightness"])
				m->SetBrightness(b);
			currSett["brightness"] = b;
			refreshScreen = true;
			break;
		}
		if (currSett["screen"]==SETTINGS) // Change selected option
		{
			if (currSett["selection"] > 0)
			{
				currSett["selection"]--;
				refreshScreen = true;
				break;
			}
		}
		
		if (currSett["screen"] > LAST_SCREEN)
			break;
		if (currSett["screen"] == FIRST_SCREEN) // Loop Around
		{
			currSett["screen"] = LAST_SCREEN;
			screenChange = true;
			refreshScreen = true;
			break;
		}
		if (currSett["screen"] > FIRST_SCREEN) // Move backward in screens
		{
			currSett["screen"]--;
			screenChange = true;
			refreshScreen = true;
			break;
		}
		
	case SW_PRESS:
		if (currSett["screen"]==SETTINGS_ENTER)
		{
			currSett["screen"] = SETTINGS; // Selection screen
			screenChange = true;
			refreshScreen = true;
			break;
		}
		if (currSett["screen"]==SETTINGS)
		{
			if (currSett["selection"]==AUTO_BRIGHT)
			{
				if (currSett["autoBrightness"])
				{
					currSett["autoBrightness"] = 0;
					matrix->SetBrightness(currSett["brightness"]);
				}
				else
				{
					currSett["autoBrightness"] = 1;
					runAutoBright = true;
				}
				runWriteConfig = true;
				refreshScreen = true;
			}
			if (currSett["selection"]==BRIGHT)
			{
				currSett["screen"] = BRIGHT_CHANGE; // Allow brightness toggle
				screenChange = true;
				refreshScreen = true;
			}
			if (currSett["selection"]==KILL)
				killSigReceived = true; // Kill program
			if (currSett["selection"]==BACK)
			{
				currSett["screen"] = SETTINGS_ENTER; // Go back to root settings screen
				screenChange = true;
				refreshScreen = true;
			}
			break;
		}
		if (currSett["screen"]==BRIGHT_CHANGE)
		{
			currSett["screen"] = SETTINGS;
			screenChange = true;
			refreshScreen = true;
			break;
		}

		if (currSett["screen"] == A_CLOCK)
		{
			currSett["24hrMode"] = !currSett["24hrMode"];
			runWriteConfig = true;
			break;
		}


		break;

	case PWR_SW_PRESS:
		currSett["screen"] = SHUTDOWN;
		screenChange = true;
		refreshScreen = true;
		fprintf(stderr, "power down\n");
		break;
	
	}

	if (screenChange)
		runWriteConfig = true;

	inputReceived = false; // Input is handled
}

void drawLoop()
{
	if (!refreshScreen)
		return;
	refreshScreen = false;
	// flushBuffAtEnd: Flag used to indicate if buffering should be handled at end of drawLoop()
	bool flushBuffAtEnd = true;

	
	switch(currSett["screen"]) // Draw appropriate data
	{


	case WEATHER1:
	{
		// Vars
		static int xPos; // For scrolling text
		bool isScrolling = false;
		if (screenChange)
		{
			screenChange = 0;
			xPos = M_WIDTH; // Start offscreen
		}

		offscreen->Clear();

		// Construct Strings
		string highAndLow = 	to_string(wd->high) + "F/" + to_string(wd->low) + "F";
		string precipChance = 	to_string(wd->precipProb) + "%";
		string currTemp = 		to_string(wd->temp) + "F";
		string appTemp = 		to_string(wd->apparentTemp) + "F";
		
		
		// Draw Weather Icon
		weatherIcons->drawCenter(wd->iconMap, offscreen, 11, 8);

		// Draw Text
		DrawTextByCenter(offscreen,  f_5x7,	 42,  3,	orange,    NULL, 	highAndLow);
		DrawTextByCenter(offscreen,	 f_5x7,	 31, 10,    skyBlue,   NULL, 	precipChance);
		DrawTextByCenter(offscreen,  f_5x7,  53, 10, 	limeGreen, NULL, 	currTemp);
		DrawTextByCenter(offscreen,  f_5x7,	 53, 17,	brightRed, NULL, 	appTemp);
		

		// Decide to scroll or fix in place the current conditions summary text
		if (getTotalWidth(f_5x7, wd->currSummary) > M_WIDTH - 6) // Too big, need to scroll
		{
			scrollTextAtCenter(xPos,offscreen,f_5x7,26,white,NULL,wd->currSummary.c_str());
			isScrolling = true;
		}
		else // Text can fit comfortably
		{
			DrawTextByCenter(offscreen, f_5x7, 31,  26, white, NULL, wd->currSummary.c_str(), 0);
		}


		offscreen = matrix->SwapOnVSync(offscreen, 1);
		flushBuffAtEnd = false;
		refreshScreen = true; // Loop Continuously

		if (isScrolling)
			usleep(SCROLL_DELAY_USEC);
		else
			usleep(0.5e6); // Reasonable update delay

	}
		break;


	case WEATHER2:
	{
		// Var declaration
		static int x1;

		if (screenChange) // Flag indicates first loop upon transition to this screen, reset at end of main
		{
			// Init positions for scrolling texts
			x1 = M_WIDTH;
			screenChange = 0;
		}

		offscreen->Clear();

		//====// Format Data
		char sunriseText	[15] = 	"";
		char sunsetText		[15] = 	"";
		
		// Sunrise/Sunset
		struct tm* 	tmStruct = localtime(&(wd->sunrise));
		strftime(sunriseText, 15, 	"SR-%-I:%M%p", tmStruct);
					tmStruct = localtime(&(wd->sunset));
		strftime(sunsetText, 15, 	"SS-%-I:%M%p", tmStruct);

		// Moon Phase
		string moonText = "MN-" + to_string(wd->moonPhase) + "%";
		// uv Index
		string uvIndexText = "UV-" + to_string(wd->uvIndex);

		string phraseText = wd->todaySummary + " //  " + wd->weekSummary;


		//====// Draw data
		scrollTextAtCenter(x1,offscreen,f_4x6, 2,white,NULL,phraseText);
		weatherIcons->drawCenter(wd->moonPhaseIcon, offscreen, 10, 13);
		DrawTextByCenter(offscreen, f_4x6, 39,  9, pureYellow, NULL, sunriseText);
		DrawTextByCenter(offscreen, f_4x6, 39, 15, orange	 , NULL, sunsetText);
		DrawTextByCenter(offscreen, f_4x6, 39, 21, skyBlue	 , NULL, moonText);
		DrawTextByCenter(offscreen, f_4x6, 39, 27, brightRed , NULL, uvIndexText);
		


		/* Handle swapping here inside this loop, for this particular state 
		   (To give instant response for state switching) */
		offscreen = matrix->SwapOnVSync(offscreen, 1);
		flushBuffAtEnd = false;
		refreshScreen = true;
		usleep(SCROLL_DELAY_USEC);

	}
		break;


	case WEATHER3:
	{
		offscreen->Clear();

		if (screenChange)
		{
			screenChange = 0;
		}

		//====// Format Data
		char humidityText	[20] = 	"";
		char visibilityText	[20] = 	"";
		char windText		[20] = 	"";
		char directionText	[20] = 	"";
		char updatedText	[20] = 	"";

		sprintf(humidityText, 		"HUM-%d%%", wd->humidity);
		sprintf(visibilityText, 	"VIS-%.1f mi", wd->visibility);
		sprintf(windText, 			"WND-%.1f mph", wd->windGust);
		sprintf(directionText,  	"DIR-%d°-%s", wd->windBearing, wd->windDir.c_str()); // ALT-0176
		// Last Updated Timestamp
		struct tm* 	tmStruct = localtime(&(wd->lastUpdated));
		strftime(updatedText, 20, 	"UP-%-m/%-d-%-I:%M%p", tmStruct);

		//====// Draw data
		DrawTextByCenter(offscreen, f_4x6, M_WIDTH/2,  2, skyBlue, NULL,   humidityText);
		DrawTextByCenter(offscreen, f_4x6, M_WIDTH/2,  8, orange	 , NULL,  visibilityText);
		DrawTextByCenter(offscreen, f_4x6, M_WIDTH/2, 14, pureGreen	 , NULL,  windText);
		DrawTextByCenter(offscreen, f_4x6, M_WIDTH/2, 20, brightRed , NULL,   directionText);
		DrawTextByCenter(offscreen, f_4x6, M_WIDTH/2, 26, pureYellow , NULL,   updatedText);


		
	}
	break;


	case WEATHER4:
	{
		offscreen->Clear();

		if (screenChange)
		{
			screenChange = 0;
		}

		//====// Format Data
		char ozoneText		[20] = 	"";
		char pressureText	[20] = 	"";
		char dewPointText	[20] = 	"";
		char cloudText		[20] = 	"";
		sprintf(ozoneText, 		"OZN-%.1f DU", wd->ozone);
		sprintf(pressureText, 	"PRS-%.2f mb", wd->pressure);
		sprintf(dewPointText, 	"DWP-%.2fF", wd->dewPoint);
		sprintf(cloudText,  	"CLD-%d%%", wd->cloudCover);


		//====// Draw data
		DrawTextByCenter(offscreen, f_4x6, M_WIDTH/2,  2, skyBlue , NULL,   cloudText);
		DrawTextByCenter(offscreen, f_4x6, M_WIDTH/2,  8, pureGreen	 , NULL,  dewPointText);
		DrawTextByCenter(offscreen, f_4x6, M_WIDTH/2, 14, orange	 , NULL,  pressureText);
		DrawTextByCenter(offscreen, f_4x6, M_WIDTH/2, 20, purple, NULL,   ozoneText);
		
		
		
	}
	break;

	case A_CLOCK:
	{
		// Static vars
		static struct timespec nextTime;
		static struct tm* timeStruct;
		static char dayText			[10];
		static char dateText		[10];
		static char yearText		[10];
		static char timeTextLine1	[15];
		static char timeTextLine2	 [5];

		if (screenChange) // Initial events
		{
			screenChange = 0;
			// Initialize time vars, set to current time
			nextTime.tv_sec = time(NULL);
			//nextTime.tv_sec = 1565001008; // Bogus morning Test Time
			//nextTime.tv_sec = 1565047808; // evening
			nextTime.tv_nsec = 0;
		}

		offscreen->Clear();

		static int cX = M_WIDTH/2 - 16;
		static int cY = M_HEIGHT/2 - 1;
		static int r = M_HEIGHT/2 - 1;

		Color cir = darkBlue;
		Color hr = orange;
		Color min = purple;
		Color sec = darkBlue;

		// Process Time
		timeStruct = localtime(&nextTime.tv_sec);
		strftime(dayText, 10, 		"%a",		 	timeStruct);
		strftime(dateText, 10, 		"%b %-d",		timeStruct);
		strftime(yearText, 10, 		"%Y", 		 	timeStruct);
		if (currSett["24hrMode"])
		{
			strftime(timeTextLine1, 15, "%H:%M:%S", 	timeStruct);
			strftime(timeTextLine2, 5, 	"", 			timeStruct);
		}
		else // 12hr Mode
		{
			strftime(timeTextLine1, 15, "%-I:%M:%S", 	timeStruct);
			strftime(timeTextLine2, 5, 	"%p", 			timeStruct);
		}

		//=====// Drawing
		DrawAnalogClock(offscreen,cX,cY,r,cir,hr,min,sec,timeStruct);
		DrawTextByCenter(offscreen, f_4x6, 47,  3, purple, NULL, 	dayText);
		DrawTextByCenter(offscreen, f_4x6, 47,  9, darkBlue, NULL,	dateText);
		DrawTextByCenter(offscreen, f_4x6, 47, 15, orange, NULL, 	yearText);
		DrawTextByCenter(offscreen, f_4x6, 47, 21, purple, NULL, 	timeTextLine1);
		DrawTextByCenter(offscreen, f_4x6, 47, 27, purple, NULL, 	timeTextLine2);
		
		// Sleep until system clock matches the next second tick
		if (clock_nanosleep(CLOCK_REALTIME, TIMER_ABSTIME, &nextTime, NULL) == EINTR)
		{
			// Woken by py signal, don't care, so go back to sleep
			if (readNewData != 0)
				clock_nanosleep(CLOCK_REALTIME, TIMER_ABSTIME, &nextTime, NULL);
			// Reset Time
			nextTime.tv_sec = time(NULL);
			nextTime.tv_nsec = 0;
		}
		else // Not Interrupted
		{
			offscreen = matrix->SwapOnVSync(offscreen, 1);
			nextTime.tv_sec += 1;
		}


		flushBuffAtEnd = false;
		refreshScreen = true; // Loop Continuously
		
	}
		break;


	case VOTD:
	{
		// Var declaration
		static int x1;
		if (screenChange) // Flag indicates first loop upon transition to this screen
		{
			// Init positions for scrolling texts
			x1 = M_WIDTH;
			screenChange = 0;
		}

		offscreen->Clear();
		// Write forecast data
		DrawTextCentered(offscreen,f_4x6, 5,orange,NULL,"Verse-Of-The-Day");
		scrollText(x1,offscreen,f_4x6,	 11,pureGreen,NULL,verse.c_str());

		/* Handle swapping here inside this loop, for this particular state 
		   (To give instant response for state switching) */
		offscreen = matrix->SwapOnVSync(offscreen, 1);
		flushBuffAtEnd = false;
		usleep(SCROLL_DELAY_USEC);

	}
		break;


	case SETTINGS_ENTER:
		offscreen->Clear();

		if (screenChange)
		{
			screenChange = 0;
		}

		DrawTextByCenter(offscreen, f_5x7, 41, f_5x7.baseline()-2, brightRed, NULL, "Enter",0);
		DrawTextByCenter(offscreen, f_5x7, 41, 2*f_5x7.baseline()-1, brightRed, NULL, "Settings",0);
		ifaceIcons->drawCenter(0, offscreen, 11, 8);

		break;

	case BLANK:
		offscreen->Clear();

		if (screenChange)
		{
			screenChange = 0;
		}
		break;

	case SETTINGS:
	{
		offscreen->Clear();

		if (screenChange)
		{
			screenChange = 0;
		}

		vector<string> options(settingSelections);
		
		const Color ARROW_COLOR = orange;
		const Color TEXT_COLOR = darkBlue;
		
		// Dynamic Brightness Text
		if (currSett["autoBrightness"])
			options[0].append("ON");
		else
			options[0].append("OFF");

		const int vertSpacing = 1;
		const int fromTop = 5;

		int y;
		uint8_t numOptions = options.size();
		for (size_t i = 0; i < numOptions; i++) // Draw each option and selection arrow
		{
			int* xBound;

			y = fromTop + i*(f_4x6.baseline() + vertSpacing);
			xBound = DrawTextCentered(offscreen,f_4x6,y,TEXT_COLOR,NULL, options[i],0);

			if (i == currSett["selection"]) // Draw Selection Arrows
			{
				int x1 = xBound[0] - f_4x6.CharacterWidth('>');
				DrawText(offscreen,f_4x6,x1,y,ARROW_COLOR,NULL,">");
				DrawText(offscreen,f_4x6,xBound[1]+1,y,ARROW_COLOR,NULL,"<");
			}
		}
	}
		break;


	case BRIGHT_CHANGE:
	{
		offscreen->Clear();

		if (screenChange)
		{
			screenChange = 0;
		}

		int b = currSett["brightness"];
		string bText = "ManBrt=";
		DrawTextCentered(offscreen,f_4x6,f_4x6.baseline(),darkBlue,NULL,bText);
		DrawTextCentered(offscreen,f_4x6,2*f_4x6.baseline()+1,orange,NULL,to_string(b));
		
		
	}
		break;


	case SHUTDOWN:
		offscreen->Clear();

		if (screenChange)
		{
			screenChange = 0;
		}

		weatherIcons->drawCenter(26, offscreen, 11, 8);
		DrawTextCentJust(offscreen, f_5x7, 41, f_5x7.baseline()+1,   blue, NULL, "Powering");
		DrawTextCentJust(offscreen, f_5x7, 42, 2*f_5x7.baseline()+1, blue, NULL, "down");
		break;
	}
	

	// Page flip - Tied to a fraction of the refresh rate
	if (flushBuffAtEnd)
		offscreen = matrix->SwapOnVSync(offscreen, 1);
	
}


void updateWeather()
{
	if (readNewData == 2) // Signal from Python Script
	{
		readNewData = 0;
		refreshScreen = true;
		wd->readFromFile(WEATHER_FILE);
		cerr << "Read weather data\n";
		//wd->printDebugData();
	}
}


void updateVerse()
{
	// Signal from Python Script
	if (readNewData==1)
	{
		readNewData = 0;
		refreshScreen = true;
		readVerse();
		cerr << "Read verse data\n";
	}
}


void readVerse()
{
	if (fileExists(VERSE_FILE))
	{
		std::ifstream inFile(VERSE_FILE.c_str());
		getline(inFile,verse);
		inFile.close();

		//cerr << verse << "\n";
	}
}


static void killHandler(int signo) 
{
	fprintf(stderr,"Signal Received: %d\n",signo);
	killSigReceived = true;
}


static void InputWaker(int signo)
{
	fprintf(stderr,"Input Event Logged\n");
	inputReceived = true;
}


static void PyHandler(int signo)
{
	cerr << signo << " receieved\n";
	if (signo == SIGRTMIN) // Verse Data
		readNewData = 1;
	if (signo == SIGRTMIN + 1) // Weather Data
		readNewData = 2;
}


void loadFonts()
{
	vector<string> completePaths;
	for (int i = 0; i < NUM_FONTS; i++)
	{
		completePaths.push_back(FONT_FP + FONT_FILES[i]);
	}
	atari.LoadFont(completePaths[0].c_str());
	f_4x6.LoadFont(completePaths[1].c_str());
	f_5x7.LoadFont(completePaths[2].c_str());
	f_6x9.LoadFont(completePaths[3].c_str());
	f_6x12.LoadFont(completePaths[4].c_str());
}


inline bool fileExists (const string& name) 
{
  struct stat buffer;   
  return (stat (name.c_str(), &buffer) == 0); 
}


void DrawTextCentJust(FrameCanvas* c, const Font &font, int x, int y, const Color &color, const Color* backColor,
					  const string text, int kOff)
{
	int width = getTotalWidth(font, text, kOff);
	
	// Calculate left x coord from center x pos
	int xBL = x - round(width/2.0);

	DrawText(c, font, xBL, y, color, backColor, text.c_str(), kOff);
}


void DrawTextRightJust(FrameCanvas* c, const Font &font, int x, int y, const Color &color, const Color* backColor,
					  const string text, int kOff)
{
	int width = getTotalWidth(font, text, kOff);
	
	// Calculate left x coord from far right x pos
	int xBL = x - width + 1; // +1 is a correction

	DrawText(c, font, xBL, y, color, backColor, text.c_str(), kOff);
}


int* DrawTextCentered(FrameCanvas *c, const Font &font, int y, const Color &color, const Color *backColor,
                      const string text, int kOff)
{
	static int xBoundaries[2];

	int totalWidth = getTotalWidth(font,text,kOff);
	// Calculate coord to center text
	double xDouble = (static_cast<double>(M_WIDTH)/2) - static_cast<double>(totalWidth)/2;
	int xInt = round(xDouble);

	xBoundaries[0] = xInt;
	xBoundaries[1] = round(M_WIDTH/2.0 + totalWidth/2.0); // FP div, then round to int

	//cerr << cWidth << " " << numChars << " " << totalWidth << " " << xDouble << " " << xInt << endl;
	
	DrawText(c, font, xInt, y, color, backColor, text.c_str(), kOff);

	return xBoundaries;
}


void DrawTextByCenter(FrameCanvas* c, const Font &font, int x, int y, const Color &color, const Color* backColor,
					  const string text, int kOff)
{
	int width = getTotalWidth(font, text, kOff);
	int height = font.height();

	// Calculate bottom-left coords
	int xBL = x - round(width/2.0) + 1; // +1 is a correction
	int yBL = y + round(height/2.0);

	DrawText(c, font, xBL, yBL, color, backColor, text.c_str(), kOff);
}

int* DrawTextMultiColorCentered(FrameCanvas* c, const Font &font, int y, const vector<Color>, const Color* backColor,
								const vector<string> strings)
{
	return NULL;
}

int getTotalWidth(const Font &font, const string text, int kOff)
{
	int cWidth = font.CharacterWidth('A'); // Includes 1 pixel offset at right for each char (in most fonts)
	int numChars = text.length();
	// Don't include offset for last char
	int totalWidth = cWidth*numChars + (kOff)*(numChars - 1) - 1; // -1 is a correction
	return totalWidth;
}


void scrollText(int &x, FrameCanvas* c, const Font &font, int y, const Color &color, const Color* backColor,
				const string text, int kOff)
{
	// Total number of pixels horizontally
	int totalWidth = getTotalWidth(font,text,kOff);

	DrawText(c,font,x,y,color,backColor,text.c_str(),kOff);

	if (x < -totalWidth) // When text is off screen
		x = M_WIDTH; // Reset to far right
	else
		x--;

	refreshScreen = true; // run drawLoop() continuously
}


void scrollTextAtCenter(int &x, FrameCanvas* c, const Font &font, int y, const Color &color, const Color* backColor,
						const string text, int kOff)
{
	// Total number of pixels horizontally
	int totalWidth = getTotalWidth(font,text,kOff);
	int yBL = y + round(font.height()/2.0);

	DrawText(c,font,x,yBL,color,backColor,text.c_str(),kOff);

	if (x < -totalWidth) // When text is off screen
		x = M_WIDTH; // Reset to far right
	else
		x--;

	refreshScreen = true; // run drawLoop() continuously
}


void DrawAnalogClock(FrameCanvas* c, int centerX, int centerY, int radius, Color &cir, Color &hr, Color &min,
					 Color &sec, struct tm* timeStruct, bool smallClock)
{
	int hrInt = timeStruct->tm_hour;
	if (hrInt >= 12) // Adjust for PM
		hrInt -= 12;
	int minInt = timeStruct->tm_min;
	int secInt = timeStruct->tm_sec;

	double thetaHr = 2*PI*hrInt/12 + (PI/6)*minInt/60; // Add the min fraction for one "slice" (PI/6)
	double thetaMin = 2*PI*minInt/60;
	double thetaSec = 2*PI*secInt/60;

	// Length of hands
	int rHrHand = round(radius*0.5);
	int rMinHand = round(radius*0.7);
	int rSecHand = round(radius*0.7);

	// Points on co-centric circles
	int hrPx = round(centerX + rHrHand*sin(thetaHr));
	int hrPy = round(centerY - rHrHand*cos(thetaHr));
	int minPx = round(centerX + rMinHand*sin(thetaMin));
	int minPy = round(centerY - rMinHand*cos(thetaMin));
	int secPx = round(centerX + rSecHand*sin(thetaSec));
	int secPy = round(centerY - rSecHand*cos(thetaSec));

	// Main body of clock
	DrawCircle(c,centerX,centerY,radius,cir);
	if (!smallClock)
	{
		DrawCircle(c,centerX,centerY,radius-1,cir); // Make circle thicker
		if (radius == 15) // Fix Gaps
		{
			// Q1
			c->SetPixel(centerX+12,centerY-8,cir.r,cir.g,cir.b);
			c->SetPixel(centerX+8,centerY-12,cir.r,cir.g,cir.b);
			// Q2
			c->SetPixel(centerX-12,centerY-8,cir.r,cir.g,cir.b);
			c->SetPixel(centerX-8,centerY-12,cir.r,cir.g,cir.b);
			// Q3
			c->SetPixel(centerX-12,centerY+8,cir.r,cir.g,cir.b);
			c->SetPixel(centerX-8,centerY+12,cir.r,cir.g,cir.b);
			// Q4
			c->SetPixel(centerX+12,centerY+8,cir.r,cir.g,cir.b);
			c->SetPixel(centerX+8,centerY+12,cir.r,cir.g,cir.b);
		}
	}

	// Draw inner tick marks for each hour for a large clock
	if (!smallClock)
	{
		int smallerR = radius - 1; // Determines length of tick marks
		for (int i = 0; i < 12; i++)
		{
			double theta = 2*PI*i/12;
			
			int innerX = round(centerX + (smallerR)*sin(theta));
			int innerY = round(centerY - (smallerR)*cos(theta));
			int circleX = round(centerX + (radius)*sin(theta));
			int circleY = round(centerY - (radius)*cos(theta));
			DrawLine(c,innerX,innerY,circleX,circleY,hr); // Same color as hour hand
		}
	}

	DrawLine(c,centerX,centerY,hrPx,hrPy,hr);    // Hour hand
	DrawLine(c,centerX,centerY,minPx,minPy,min); // Minute hand
	DrawLine(c,centerX,centerY,secPx,secPy,sec); // Second hand
	
}

void writeConfig()
{
	if (!runWriteConfig)
		return;
	runWriteConfig = 0;
	// Loop over each line of stored format, and update values based on map
	for (int i = 0; i < configFile.size(); i++)
	{
		if (configFile[i][0] != ';' && !configFile[i].empty()) // Skip comment/blank lines
		{
			// Example Line: key=value or var=5
			int pos = configFile[i].find_first_of('=');
			string key = configFile[i].substr(0,pos);

			unsigned char val = currSett[key];
			// Don't update to shutdown screen
			if (key=="screen" && val==SHUTDOWN)
				continue;
			string updatedLine = key + "=" + to_string(val);

			configFile[i] = updatedLine;
		}
	}

	std::ofstream fd(CONFIG_FILE.c_str());

	if (fd.good())
	{
		for (int i = 0; i < configFile.size(); i++)
		{
			fd << configFile[i];
			if (i != configFile.size() - 1)
				fd << endl;
		}

		fd.close();

		cerr << "Wrote to config file\n";
	}
}

bool readConfig()
{
	bool fileGood = true;
	std::ifstream fd(CONFIG_FILE.c_str()); // Open for reading
	
	if (!fd.good()) // file doesn't exist
		fileGood = false;
	
	if (fd.good())
	{
		string buff;

		while(fd.good()) // Write each line into config string vector
		{
			getline(fd,buff);
			configFile.push_back(buff);
		}

		// Loop over each line, parse, and add to map
		for (int i = 0; i < configFile.size(); i++)
		{
			if (configFile[i][0] != ';' && !configFile[i].empty()) // Skip comment/blank lines
			{
				// Example Line: key=value or var=5
				int pos = configFile[i].find_first_of('=');
				string key = configFile[i].substr(0,pos);
				unsigned char value = stoi(configFile[i].substr(pos+1));

				// Store in map
				currSett[key] = value;
			}
		}

		// Fix last screen in event of kill/shutdown event
		if (currSett["screen"] == SETTINGS ||
			currSett["screen"] == SHUTDOWN)
		{
			currSett["screen"] = defSett["screen"];
		}
		cerr << "Read config file\n";
	}

	return fileGood;
}

void setDefaultConfig()
{
	defSett["brightness"] = 50;
	defSett["autoBrightness"] = 0;
	defSett["24hrMode"] = 0;
	// screen: State variable to indicate which screen to draw
	defSett["screen"] = 0;
	// selection: State variable used on the setting screen
	defSett["selection"] = 0;
	// rampTime: Auto brightness ramp time in minutes
	defSett["rampTime"] = 30;
	defSett["minBright"] = 2;
	defSett["maxBright"] = 50;

	for (auto i : defSett)
		currSett[i.first] = i.second;

}

void printConfig()
{
	cerr << "Current Settings:\n";
	for (auto i : currSett)
	{
		cerr << i.first << ":" << static_cast<int>(i.second) << endl;
	}
}

void autoBrightness()
{
	if (!currSett["autoBrightness"])
		return;
	// Update at max every 1 sec
	static time_t lastUpdated = time(NULL);
	if (difftime(time(NULL), lastUpdated) < 1 && !runAutoBright)
		return;

	runAutoBright = false;

	// Get Times
	time_t rawTime = time(NULL);
	//rawTime = testTime;

	struct tm* timeInfo = localtime(&rawTime);
	uint32_t currSec = timeInfo->tm_hour*3600 + timeInfo->tm_min*60 + timeInfo->tm_sec;
	timeInfo = localtime(&(wd->sunrise));
	uint32_t sunriseSec = timeInfo->tm_hour*3600 + timeInfo->tm_min*60 + timeInfo->tm_sec;
	timeInfo = localtime(&(wd->sunset));
	uint32_t sunsetSec = timeInfo->tm_hour*3600 + timeInfo->tm_min*60 + timeInfo->tm_sec;

	float rampRate = (currSett["maxBright"]-currSett["minBright"])/(currSett["rampTime"]*60.0); // in BU/sec

	uint32_t srLim[2];
	uint32_t ssLim[2];
	srLim[0] = sunriseSec - currSett["rampTime"]*30; // Half of ramp time, in sec
	srLim[1] = sunriseSec + currSett["rampTime"]*30;
	ssLim[0] = sunsetSec - currSett["rampTime"]*30;
	ssLim[1] = sunsetSec + currSett["rampTime"]*30;

	uint8_t currB = matrix->brightness();
	uint8_t b = currB;
	// Early Morning
	if (currSec < srLim[0])
		b = currSett["minBright"];
	// Ramp Up - Linear
	if (currSec >= srLim[0] && currSec <= srLim[1])
		b = round(rampRate*(currSec - srLim[0]) + currSett["minBright"]);
	// Daytime
	if (currSec > srLim[1] && currSec < ssLim[0])
		b = currSett["maxBright"];
	// Ramp Down - Linear
	if (currSec >= ssLim[0] && currSec <= ssLim[1])
		b = round(-rampRate*(currSec - ssLim[0]) + currSett["maxBright"]);
	// Late Night
	if (currSec > ssLim[1])
		b = currSett["minBright"];

	if (currB != b)
	{
		matrix->SetBrightness(b);
		refreshScreen = true;
	}

	lastUpdated = time(NULL);
	cerr << "autoBrightness sampled\n";
}