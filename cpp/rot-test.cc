/*
	Title: rot-test.cc
	Author: Garrett Carter
	Date: 5/16/19
	Purpose: Test driver for RotInput Class
*/

#include "RotInput.h"
#include "graphics.h"
#include <cstdio>
#include <cstdlib>
#include <signal.h>
#include <fstream>
#include <cstring>

using namespace rgb_matrix;

//// CONSTANTS
const int CLK_PIN = 25; // Output A - Phys pin 25
const int DT_PIN = 9;  // Output B - Phys pin MISO
const int SW_PIN = 10; // Phys Pin MOSI
const int PWR_SW_PIN = 3; // SCL pin (also wakeup)
const std::string fontFP = "../rpi-rgb-led-matrix/fonts";
const int MAX_HEIGHT = 31; // coords
const int MAX_WIDTH = 63;

//// GLOBALS
volatile bool interruptReceived = false; // may change asynchronously

//// FUNCTION PROTOTYPES
static void InterruptHandler(int signo) { interruptReceived = true; }
static void BasicHandler(int signo) { return; }

int main(int argc, char** argv)
{
	fprintf(stderr,"PID: %d\n",::getpid());
	signal(SIGTERM, InterruptHandler);
	signal(SIGINT, InterruptHandler);
	signal(SIGUSR1, BasicHandler);
	
	RGBMatrix::Options defaults;
	RuntimeOptions rtOps;
	
	defaults.rows = MAX_HEIGHT + 1;
	defaults.cols = MAX_WIDTH + 1;
	defaults.brightness = 50;
	
	rtOps.drop_privileges = 0; // Don't drop root
	if (argc > 1 && strcmp(argv[1], "-d") == 0) // Daemon flag
	{
		rtOps.daemon = 1; // Daemonize
	}
	
	RGBMatrix* matrix = rgb_matrix::CreateMatrixFromOptions(defaults, rtOps);
	if (matrix == NULL)
		return 1;
	
	fprintf(stderr,"Matrix created\n");
	
	// Load font
	Font smallFont;
	std::string sf = fontFP + "/5x7.bdf";
	
	if(!smallFont.LoadFont(sf.c_str()))
	{
		fprintf(stderr,"Error loading font.\n");
		return 1;
	}
	
	char pid[10];
	sprintf(pid,"PID: %d",::getpid());
	char welcome[] = "Running...";
	Color purple(255,0,255);
	Color black(0,0,0);
	
	//// Draw static text
	DrawText(matrix, smallFont, 1, smallFont.baseline(), purple, NULL, welcome, 0);
	DrawText(matrix, smallFont, 1, MAX_HEIGHT, purple, NULL, pid, 0);
	
	
	RotInput* Input = new RotInput(pthread_self(),CLK_PIN, DT_PIN, SW_PIN, matrix, PWR_SW_PIN);
	
	fprintf(stderr,"Input object created\n");
	
	int oldCount = -1;
	int count = 0;
	char countTxt[15];
	unsigned char event;
	while(!interruptReceived)
	{
		event = Input->getEvent();
		
		switch (event)
		{
			case DIR_CW:
				count++;
				break;
			case DIR_CCW:
				count--;
				break;
			case SW_PRESS:
				count = 0;
				break;
			case PWR_SW_PRESS:
				interruptReceived = true;
				fprintf(stderr, "power down\n");
				system("sudo shutdown -h now");
				break;
			
		}
		
		if (oldCount != count)
		{
			fprintf(stderr,"Count: %d\n",count);
			
			sprintf(countTxt,"Count: %d",count);
			DrawText(matrix, smallFont, 1, smallFont.baseline()*2+1, purple, &black, "               ", 0);
			DrawText(matrix, smallFont, 1, smallFont.baseline()*2+1, purple, NULL, countTxt, 0);
		}
		oldCount = count;
	}
	
	delete Input;
	matrix->Clear();
	delete matrix;
	
	fprintf(stderr,"Cleanup completed\n");
	
	// Write PID to file
	std::ofstream outFile("../out/pid_test.txt");
	if (outFile.good()) // successful open
	{
		outFile << ::getpid();
		outFile.close();
	}
	return 0;
}