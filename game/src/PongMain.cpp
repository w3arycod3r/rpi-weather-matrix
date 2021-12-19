/*
	Title: PongMain.cpp
	Author: Garrett Carter
	Date: 10/5/19
	Purpose: Main file for a Pong recreation using the rgbmatrix library, joystick and keyboard input.
*/

#include <unistd.h>
#include <signal.h>
#include <cmath>
#include <chrono>
#include "led-matrix.h"
#include "graphics.h"

#include "InputDevices.h"
#include "PongObjects.h"

using namespace std::chrono_literals;
using namespace rgb_matrix;
using std::cerr; using std::endl; using std::setw;

#define M_WIDTH 64
#define M_HEIGHT 32
#define KEYBOARD_MODE 0
#define TWO_JOY_MODE 1
#define MAX_CONN_TRIES 5
const string BY_ID = "/dev/input/by-id/";

const string KEYBOARD_PATH = BY_ID + "usb-Logitech_USB_Receiver-if02-event-mouse"; // Wireless Logitech Keyboard
//const string KEYBOARD_PATH = BY_ID + "usb-DELL_DELL_USB_Keyboard-event-kbd"; // Wired Dell Keyboard
const string JOY0_MAC = "00:19:C1:CB:BD:73"; // Sixaxis
const string JOY1_MAC = "00:23:06:A5:6C:F1"; // DualShock 3

InputDevice joystick0, joystick1, keyboard;

int playMode = KEYBOARD_MODE;
int effectID[10];
void setupEffects() {}
void waitForInputDevices();


Color red(0xff, 0, 0);
Color green(0, 0xff, 0);
Color blue(0, 0, 0xff);


// matrix: Main matrix obj from library
RGBMatrix* matrix;
// offscreen: Secondary canvas used for double buffering
FrameCanvas* offscreen;

GameWorld world;

volatile bool exitGracefully = false;
static void killHandler(int signo);

// we use a fixed timestep of 1 / (60 fps) ~= 16 milliseconds
constexpr std::chrono::nanoseconds timestep(16ms);

void processInput();
void update();
void draw(float alpha);

int main(int argc, char** argv)
{
    // SIGNAL HANDLERS
	signal(SIGTERM, killHandler);
	signal(SIGINT, killHandler);

	waitForInputDevices();

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

	matrix->Fill(0, 0, 255);

	using clock = std::chrono::high_resolution_clock;
 	std::chrono::nanoseconds lag(0ns);
  	auto time_start = clock::now();

	// Game Loop: Fixed Time Steps with Variable draw Rates using Interpolation.
    while(!exitGracefully)
    {
		auto delta_time = clock::now() - time_start;
		time_start = clock::now();
		lag += std::chrono::duration_cast<std::chrono::nanoseconds>(delta_time);

		processInput();

		// update game logic as lag permits
		while(lag >= timestep) 
		{
			lag -= timestep;
			update(); // update at a fixed rate each time
		}

		// calculate how close or far we are from the next timestep
		float alpha = lag.count() / timestep.count();

		draw(alpha);

    }


	// Cleanup
	matrix->Clear();
	delete matrix;

    return 0;
}

static void killHandler(int signo) 
{
	fprintf(stderr,"\nSignal Received: %d\n",signo);
	exitGracefully = true;
}

void draw(float alpha)
{
	world.draw(offscreen, alpha);
	offscreen = matrix->SwapOnVSync(offscreen, 1);
}

void update()
{
	world.update();
}

void waitForInputDevices()
{

if (playMode == KEYBOARD_MODE)
{
	keyboard.openPath(KEYBOARD_PATH, DEV_KEYBOARD);

	uint8_t numTries = 0;

	while(!keyboard.isFound())
	{
		numTries++;
		if (numTries > MAX_CONN_TRIES)
		{
			cerr << "Input connection timeout\n";
			exit(1);
		}

		if (!keyboard.isFound())
			cerr << "Please connect Keyboard 0...\n";

		usleep(2e6);

		keyboard.retryConnection();

		if (exitGracefully)
			exit(0);
	}

	cerr << "Keyboard 0 connected\n";
	keyboard.disableAutoRepeat();
	return;
}


if (playMode == TWO_JOY_MODE)
{
	joystick0.openPS3(JOY0_MAC);
	joystick1.openPS3(JOY1_MAC);

	uint8_t numTries = 0;

	while(!joystick0.isFound() || !joystick1.isFound())
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
		usleep(2e6);

		joystick0.retryConnection();
		joystick1.retryConnection();

		if (exitGracefully)
			exit(0);
	}

	setupEffects();

	cerr << "Joystick 0 and Joystick 1 connected\n";
	return;
}

}

void processInput()
{
	// Attempt to sample events from the devices
	InputEvent 	event[2];
	uint8_t 	status[2];

	if (playMode == KEYBOARD_MODE)
	{
		status[0] = keyboard.sample(&event[0]);

		// Keyboard 0
		if (status[0] == 1)
		{
			printf("Lost Keyboard 0\n");
			exitGracefully = true;
		}
		else if (status[0] == 2) // Good Sample
		{
			world.input(&event[0]);
		}
	}

	if (playMode == TWO_JOY_MODE)
	{
		status[0] = joystick0.sample(&event[0]);
		status[1] = joystick1.sample(&event[1]);


		// Joystick 0
		if (status[0] == 1)
		{
			printf("Lost Joystick 0\n");
			exitGracefully = true;
		}
		else if (status[0] == 2)
		{
			world.input(&event[0]);
		}


		// Joystick 1
		if (status[1] == 1)
		{
			printf("Lost Joystick 1\n");
			exitGracefully = true;
		}
		else if (status[1] == 2)
		{
			world.input(&event[1]);
		}
	}

}