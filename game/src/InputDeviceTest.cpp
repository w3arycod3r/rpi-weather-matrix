/*
	Title: InputDeviceTest.cpp
	Author: Garrett Carter
	Date: 9/30/19
	Purpose: Test PS3 Controller and keyboard input using InputIndicators
*/

#include <unistd.h>
#include <signal.h>
#include <cmath>
#include "led-matrix.h"
#include "graphics.h"

#include "InputDevices.h"
#include "GameObjects.h"



using namespace rgb_matrix;
using std::cerr; using std::endl; using std::setw;
using std::vector;

const int M_WIDTH = 64;
const int M_HEIGHT = 32;
const int MAX_CONN_TRIES = 5;
const string BY_ID = "/dev/input/by-id/";


const string KEYBOARD_PATH = BY_ID + "usb-Logitech_USB_Receiver-if02-event-mouse"; // Wireless Logitech Keyboard
//const string KEYBOARD_PATH = BY_ID + "usb-DELL_DELL_USB_Keyboard-event-kbd"; // Wired Dell Keyboard
const string JOY0_MAC = "00:19:C1:CB:BD:73"; // Sixaxis
const string JOY1_MAC = "00:23:06:A5:6C:F1"; // DualShock 3

Color red(0xff, 0, 0);
Color green(0, 0xff, 0);
Color blue(0, 0, 0xff);

// matrix: Main matrix obj from library
RGBMatrix* matrix;
// offscreen: Secondary canvas used for double buffering
FrameCanvas* offscreen;


InputIndicator Controller0(M_WIDTH/2, M_HEIGHT/2, 0, 0);
InputIndicator Controller1(M_WIDTH/2, M_HEIGHT/2, M_WIDTH/2, 0);

InputDevice joystick0, joystick1, keyboard0;
int effectID[10];
void setupEffects();

volatile bool exitGracefully = false;

static void killHandler(int signo);

void waitForInputDevices();

int main(int argc, char** argv)
{
	// SIGNAL HANDLERS
	signal(SIGTERM, killHandler);
	signal(SIGINT, killHandler);

	waitForInputDevices();
	// After this, we know all Input Devices are good.
	setupEffects();

	// Create Matrix Object
	RGBMatrix::Options defaults;
	RuntimeOptions rtOps;

	defaults.rows = M_HEIGHT;
	defaults.cols = M_WIDTH;
	defaults.brightness = 50;
	
	matrix = CreateMatrixFromOptions(defaults, rtOps);
	if (matrix == NULL)
	{
		cerr << "Matrix obj creation failed.\n";
		exit(1);
	}

	offscreen = matrix->CreateFrameCanvas();

	while (!exitGracefully)
	{
		// Restrict rate
		usleep(1000);

		// Attempt to sample events from the devices
		InputEvent 	event[3];
		uint8_t 	status[3];

		status[0] = joystick0.sample(&event[0]);
		status[1] = joystick1.sample(&event[1]);
		status[2] = keyboard0.sample(&event[2]);

		// Joystick 0
		if (status[0] == 1)
		{
			printf("Lost Joystick 0\n");
			exitGracefully = true;
		}
		else if (status[0] == 2)
		{
			Controller0.input(&event[0]);

			if (event[0].isButton())
			{
				printf("Joy0: Button %u is %s\n", event[0].jse.number, event[0].jse.value == 0 ? "up" : "down");
			}
			else if (event[0].isAxis())
			{
				//printf("Joy0: Axis %u is at position %d\n", event[0].jse.number, event[0].jse.value);
			}
		}

		// Joystick 1
		if (status[1] == 1)
		{
			printf("Lost Joystick 1\n");
			exitGracefully = true;
		}
		else if (status[1] == 2)
		{
			Controller1.input(&event[1]);

			if (event[1].isButton())
			{
				printf("Joy1: Button %u is %s\n", event[1].jse.number, event[1].jse.value == 0 ? "up" : "down");
			}
			else if (event[1].isAxis())
			{
				//printf("Joy1: Axis %u is at position %d\n", event[1].jse.number, event[1].jse.value);
			}

			if (event[1].isButtonDown(PS3_BTN_SQUARE))
			{
				joystick1.playEffect(effectID[0]);
			}

			if (event[1].isButtonDown(PS3_BTN_CIRCLE))
			{
				joystick1.playEffect(effectID[1]);
			}

			if (event[1].isButtonDown(PS3_BTN_TRIANGLE))
			{
				joystick1.stopEffect(1);
			}

		}

		// Keyboard 0
		if (status[2] == 1)
		{
			printf("Lost Keyboard 0\n");
			exitGracefully = true;
		}
		else if (status[2] == 2)
		{
			Controller0.input(&event[2]);
			Controller1.input(&event[2]);

			if (event[2].isAutoRepeat())
				cerr << event[2] << endl;
		}

		// Draw
		offscreen->Clear();
		Controller0.draw(offscreen);
		Controller1.draw(offscreen);
		offscreen = matrix->SwapOnVSync(offscreen, 1);
	}

	// Cleanup
	matrix->Clear();
	delete matrix;
}

static void killHandler(int signo) 
{
	fprintf(stderr,"Signal Received: %d\n",signo);
	exitGracefully = true;
}

void waitForInputDevices()
{
	joystick0.openPS3(JOY0_MAC);
	joystick1.openPS3(JOY1_MAC);
	keyboard0.openPath(KEYBOARD_PATH, DEV_KEYBOARD);

	uint8_t numTries = 0;

	while(!joystick0.isFound() || !joystick1.isFound() || !keyboard0.isFound())
	{
		numTries++;
		if (numTries > MAX_CONN_TRIES)
		{
			cerr << "Input connection timeout\n";
			exit(1);
		}

		if (!joystick0.isFound())
			cerr << "Please connect Joystick 0...\n";
		if (!joystick1.isFound())
			cerr << "Please connect Joystick 1...\n";
		if (!keyboard0.isFound())
			cerr << "Please connect Keyboard 0...\n";
		usleep(2e6);

		joystick0.retryConnection();
		joystick1.retryConnection();
		keyboard0.retryConnection();

		if (exitGracefully)
			exit(0);
	}

	cerr << "Joystick 0 and Joystick 1 connected\n";
	cerr << "Keyboard 0 connected\n";
	keyboard0.disableAutoRepeat();
	return;
}

void setupEffects()
{
	memset(effectID, 0, sizeof(effectID)); // Init array with zeros
	// Square
	effectID[0] = joystick1.setupEffect(10, 0, 1000);
	// Circle
	effectID[1] = joystick1.setupEffect(0, 100, 1000);
}