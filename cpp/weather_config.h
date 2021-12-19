/*
	Title: weather-config.cc
	Author: Garrett Carter
	Date: 6/28/19
	Purpose: Header file for weather-disp. Contains global var declarations, function prototypes, and lots of comments.
*/

#ifndef WEATHER_CONFIG_H
#define WEATHER_CONFIG_H

//=====// Libraries
#include "led-matrix.h"
#include "graphics.h"
#include "Weather.h"
#include "RotInput.h"
#include "ppm.h"
#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <pthread.h>
#include <fstream>
#include <cstring>
#include <sys/stat.h>
#include <cmath>
#include <iostream>
#include <unordered_map>


//=====// NAMESPACES
using namespace rgb_matrix;
using std::string; using std::vector; using std::cerr; using std::to_string;
using std::unordered_map;
using std::endl;


//=====// CONSTANTS
const string SHARE_DIR = "../out/"; // Shared Output files of programs
const string WEATHER_FILE = SHARE_DIR + "weather_data.txt";
const string PID_FILE = SHARE_DIR + "weather_pid.txt";
const string CONFIG_FILE = SHARE_DIR + "weather-disp.cfg";
const string VERSE_FILE = SHARE_DIR + "verse.txt";
const int SCROLL_DELAY_USEC = 25000;
const double PI = 3.14159265358979323846;
// Matrix Dimensions
const int M_WIDTH = 64;
const int M_HEIGHT = 32;


//=====// INPUT PINS
const int CLK_PIN = 25; // Output A - Phys pin 25
const int DT_PIN = 9;  // Output B - Phys pin MISO
const int SW_PIN = 8; // Phys Pin CE0 - High Pullup
const int PWR_SW_PIN = 3; // SCL pin (also wakeup)



/* 
 * //====// SCREEN STATE CONSTANTS
 * Functionally, this program loops from FIRST_SCREEN to LAST_SCREEN
 * To add a new screen, make a const for it, increment LAST_SCREEN, then add a draw loop for it.
 */
const uint8_t FIRST_SCREEN = 0; const uint8_t LAST_SCREEN = 7;
const uint8_t WEATHER1 = 0; const uint8_t WEATHER2 = 1; const uint8_t WEATHER3 = 2;
const uint8_t WEATHER4 = 3;
const uint8_t A_CLOCK = 4;
const uint8_t VOTD = 5;
const uint8_t SETTINGS_ENTER = 6;
const uint8_t BLANK = 7;
const uint8_t SHUTDOWN = 100; const uint8_t SETTINGS = 101;
const uint8_t BRIGHT_CHANGE = 102;



//=====// SELECTION CONSTANTS
const int AUTO_BRIGHT = 0; const int BRIGHT = 1; const int KILL = 2; const int BACK = 3;
// settingSelections: Holds the options listed on the settings screen for config
vector<string> settingSelections = {"AutBrt=","ChgBrt","KillProg","GoBck"};

//=====// GLOBAL VARS & FLAGS

// currSett: map used for various settings in the program, and saving and loading config file
unordered_map<string, unsigned char> currSett;
// defSett: map used for default settings when not specified in config file
unordered_map<string, unsigned char> defSett;
// configFile: vector used for storing lines to be rewritten upon save
vector<string> configFile;

//===// ISR Flags (Interrupt Service Routines)

// killSigReceived: ISR flag used to indicate that the program should be killed
volatile bool killSigReceived = false;
/*	readNewData: ISR flag to indicate which new data to read
	0 = None
	1 = Verse Data
	2 = Weather Data
 */
volatile int readNewData = 0; 
// inputReceived: ISR flag to indicate an input event was just received
volatile bool inputReceived = false;

// These flags should not be reset until action is taken on them

// refreshScreen:	flag used to indicate that drawLoop() should be executed in the next program loop
bool refreshScreen = true;
// screenChange:	flag to indicate from inputLoop to drawLoop that a screen transition has occurred (execute prelim events for the screen)
bool screenChange = true;
// runWriteConfig:	flag to indicate that writeConfig() should be executed in next loop
bool runWriteConfig = false;
// runAutoBright:	flag to indicate that autoBrightness() routine should be executed immediately in next loop
bool runAutoBright = true;


// testTime:		Bogus time used for testing
time_t testTime = 0;


// verse: String used to hold verse for VOTD screen
string verse = "No Verse Loaded";
// wd: WeatherData object to hold data read in from weather file
WeatherData* wd = new WeatherData;



// matrix: Main matrix obj from library
RGBMatrix* matrix;
// offscreen: Secondary canvas used for double buffering
FrameCanvas* offscreen;
// Input: RotInput obj used to run input thread for Rotary Encoder
RotInput* Input;


//=====// GLOBAL COLORS (32 color palette)
Color red(0xbe,0x26,0x33);			// 0
Color darkRed(0x73,0x29,0x30);
Color pink(0xf7,0x8e,0xd6);
Color salmon(0xe0,0x6f,0x8b);
Color purple(0xcb,0x43,0xa7);
Color lavender(0x99,0x64,0xf9);
Color darkPurple(0x34,0x2a,0x97);
Color blue(0x22,0x5a,0xf6);
Color midnightBlack(0x1b,0x26,0x32);
Color skyBlue(0x31,0xa2,0xf2);
Color darkBlue(0x00,0x57,0x84);
Color slate(0x65,0x6d,0x71);
Color periwinkle(0xb2,0xdc,0xef);
Color shadow(0x2f,0x48,0x4e);
Color teal(0x14,0x80,0x7e);
Color arctic(0x15,0xc2,0xa5);		// 15
Color forestGreen(0x11,0x5e,0x33);
Color green(0x44,0x89,0x1a);
Color limeGreen(0xa3,0xce,0x27);
Color darkGray(0x52,0x4f,0x40);
Color fawn(0xad,0x9d,0x33);
Color blonde(0xf7,0xe2,0x6b);
Color gold(0xfa,0xb4,0x0b);
Color brown(0x49,0x3c,0x2b);
Color tanCol(0xa4,0x64,0x22);
Color peach(0xeb,0x89,0x31); 		// 25
Color cantaloupe(0xf4,0xb9,0x90);
Color orange(0xec,0x47,0x00);
Color white(0xff,0xff,0xff);
Color lightGray(0xcc,0xcc,0xcc);
Color gray(0x9d,0x9d,0x9d);
Color black(0x00,0x00,0x00); 		//31
// Extras
Color brightRed(0xff,0x06,0x00);
Color pureRed(0xff,0x00,0x00);
Color pureYellow(0xff,0xff,0x00);
Color pureGreen(0x00,0xff,0x00);


//=====// GLOBAL FONTS
// To add font: add the filename in FONT_FILES, create a named font obj, and then assign in the loadFonts() function
const string FONT_FP = "../rpi-rgb-led-matrix/fonts/";
const vector<string> FONT_FILES = {"atari-small.bdf", "4x6.bdf", "5x7.bdf", "6x9.bdf", "clR6x12.bdf"};
const int NUM_FONTS = FONT_FILES.size();
Font atari, f_4x6, f_5x7, f_6x9, f_6x12;


//=====// ANIMATIONS & ICONS
// To add set of icons, follow the pattern set below. Don't forget to delete the Frames object at end of main()
const string IMAGES_DIR = "../img/";

// Weather Icons
const string WEATHER_ICN_FP = IMAGES_DIR + "weather/";
const int WEATHER_ICN_CNT = 38;
Frames* weatherIcons = new Frames(WEATHER_ICN_FP, WEATHER_ICN_CNT);
// Interface Icons
const string IFACE_ICN_FP = IMAGES_DIR + "interface/";
const int IFACE_ICN_CNT = 1;
Frames* ifaceIcons = new Frames(IFACE_ICN_FP, IFACE_ICN_CNT);




//=====// FUNCTION PROTOTYPES

//===// Interrupt (Signal) Handlers

// killHandler(): Sets the flag that kills the program
static void killHandler(int signo);
/* 	InputWaker():	Just a basic handler to wake any currently sleeping functions, called by a signal
					coming from RotInput thread to ensure quick response to input events  
*/
static void InputWaker(int signo);
// PyHandler(): Handles signals from python scripts by setting appropriate flags.
static void PyHandler(int signo);


// drawLoop(): Main draw loop, handles drawing each screen
void drawLoop();
// inputLoop(): Main input loop, handles screen switching based on input from RotInput thread
void inputLoop();


// updateWeather(): Call readFile on WeatherData object when signal receieved
void updateWeather();
// updateVerse(): Call readVerse() when signal receieved
void updateVerse();
// readVerse(): Store the data from VERSE_FILE into the global verse string
void readVerse();
// loadFonts(): Load Font objects
void loadFonts();


// fileExists(): Checks if the file exists on the disk
bool fileExists (const string& name);

// Function: DrawTextCentJust()
// Purpose:  Draws text justified at center. Coords will be at the baseline, horiz center of text.
void DrawTextCentJust(FrameCanvas* c, const Font &font, int x, int y, const Color &color, const Color* backColor,
					  const string text, int kOff = 0);
// Function: DrawTextRightJust()
// Purpose:  Draws text justified at right. Coords will be at the baseline, far right of text.
void DrawTextRightJust(FrameCanvas* c, const Font &font, int x, int y, const Color &color, const Color* backColor,
					  const string text, int kOff = 0);
/*
	Function: DrawTextCentered()
	Purpose:  Draws text centered horizontally on the screen using the parameters, no x coord necessary
	Note:	  Y coord will be at baseline of text

	Returns: an int array of size 2 holding [x1,x2], the left and right x boundaries of the text, useful for drawing something else
		on either side, like selection arrows in a different color
		**This array is static and overwritten with each call to this function**
 */
int* DrawTextCentered(FrameCanvas *c, const Font &font, int y, const Color &color, const Color *backColor,
                      const string text, int kOff = 0);
// DrawTextByCenter(): Draws text with the given parameters. X and Y positions will be the approximate CENTER of the text.
//					   This center is horiz and vert
void DrawTextByCenter(FrameCanvas* c, const Font &font, int x, int y, const Color &color, const Color* backColor,
					  const string text, int kOff = 0);
// DrawTextMultiColorCentered(): TODO
int* DrawTextMultiColorCentered(FrameCanvas* c, const Font &font, int y, const vector<Color>, const Color* backColor,
								const vector<string> strings);
// getTotalWidth(): Returns the total # of pixels the string will horizontally occupy, with the given kerning offset
int getTotalWidth(const Font &font, const string text, int kOff=0);
/*
	scrollText(): Handles the x-pos for moving a text string from offscreen RHS to offscreen LHS. Advances one pixel per call. Handle a scroll delay externally.
		Uses M_WIDTH constant to determine matrix width. y parameter is the baseline level (approx. bottom). Start x at M_WIDTH to be offscreen.
*/
void scrollText(int &x, FrameCanvas* c, const Font &font, int y, const Color &color, const Color* backColor,
				const string text, int kOff=0);
/*
	scrollTextAtCenter(): Handles the x-pos for moving a text string from offscreen RHS to offscreen LHS. Advances one pixel per call. Handle a scroll delay externally.
		Uses M_WIDTH constant to determine matrix width. y parameter is at the approximate CENTER of the text, like DrawTextByCenter(). Start x at M_WIDTH to be offscreen.
*/
void scrollTextAtCenter(int &x, FrameCanvas* c, const Font &font, int y, const Color &color, const Color* backColor,
						const string text, int kOff=0);
/*
	Function: DrawAnalogClock()
	Params: Colors are for main circle, hour, minute, and second hands
			Set smallClock = true to make a small clock less cluttered
 */
void DrawAnalogClock(FrameCanvas* c, int centerX, int centerY, int radius, Color &cir, Color &hr, Color &min,
					 Color &sec, struct tm* timeStruct, bool smallClock = false);


/*	Function: 	writeConfig()
	Purpose:	Writes config data to file using read in line format and settings map.
			 	Defaults not present in file prev. read will not be written.
*/
void writeConfig();
// readConfig(): Reads config data into line format vector and populates settings map
bool readConfig();
// printConfig(): Print all the settings in the map
void printConfig();
// setDefaultConfig(): Write defaultSettings into currentSettings. Call this before reading config.
void setDefaultConfig();
// autoBrightness(): Handle auto brightness ramping
void autoBrightness();


#endif // ifndef WEATHER_CONFIG_H