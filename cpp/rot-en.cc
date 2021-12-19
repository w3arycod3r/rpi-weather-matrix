/*
	Title: rot-en.cc
	Author: Garrett Carter
	Date: 5/9/19
	Purpose: Use a rotary encoder to control led-matrix library (move a pixel around)
*/

#include "led-matrix.h"

#include <unistd.h>
#include <math.h>
#include <stdio.h>
#include <signal.h>
#include <bitset>
#include <iostream>
#include <pthread.h>

using namespace rgb_matrix;

volatile bool interrupt_received = false;
static void InterruptHandler(int signo) 
{
	interrupt_received = true;
}

// Extracts pin info from the binary inputs string
bool checkPin(uint32_t inputs, int pin);

void* processInput(void* ptr);
void drawLoop(RGBMatrix* &matrix, FrameCanvas* &offscreen);

const int CLK_PIN = 26; // Output A - Phys pin 25
const int DT_PIN = 10;  // Output B - Phys pin MISO
const int SW_PIN = 11; // Phys Pin MOSI

volatile int xPos = 32;
volatile int yPos = 16;
volatile int moveDir = 0; // 0 = left-right, 1 = up-down

volatile bool refreshScreen = true;

int main(int argc, char *argv[])
{
	fprintf(stderr, "PID: %d\n", ::getpid()); // system function getpid()
	RGBMatrix::Options defaults;
	RuntimeOptions rtOps;
	
	defaults.rows = 32;
	defaults.cols = 64;
	defaults.brightness = 50;
	
	RGBMatrix* matrix = rgb_matrix::CreateMatrixFromOptions(defaults, rtOps);
	if (matrix == NULL)
		return 1;
	
	// It is always good to set up a signal handler to cleanly exit when we
	// receive a CTRL-C for instance.
	signal(SIGTERM, InterruptHandler);
	signal(SIGINT, InterruptHandler);
	
	pthread_t inputThread;
	
	pthread_create(&inputThread, NULL, processInput, (void*)matrix); // Start input proc thread
	
	// Matrix is front buffer, offscreen is back buffer
	FrameCanvas* offscreen = matrix->CreateFrameCanvas();
	
	while (!interrupt_received) 
	{
		drawLoop(matrix, offscreen);
	}
	
	fprintf(stderr, "\nExiting.\n");
	matrix->Clear();
	delete matrix;
	
	
	pthread_cancel(inputThread); // Send cancellation request to thread
	pthread_join(inputThread, NULL); // Wait for thread to cancel
	
	return 0;
}

bool checkPin(uint32_t inputs, int pin)
{
	return inputs & (1 << (pin-1)); // Extract bit #(pin-1) from the right
}

void* processInput(void* ptr)
{
	RGBMatrix* matrix = (RGBMatrix*)ptr; // Cast the void* back into a RGBMatrix*
	// Init (reserve) all available inputs
	matrix->gpio()->RequestInputs(0xffffffff);
	
	bool lastA, currA;
	bool lastSW, currSW;
	
	usleep(1e5); // Delay to allow inputs to stabilize
	uint32_t inputs = matrix->AwaitInputChange(0); // Get init state
	
	lastA = checkPin(inputs, CLK_PIN);
	lastSW = checkPin(inputs, SW_PIN);
	
	//fprintf(stderr,"init input, lastA = %d, lastSW = %d\n",lastA,lastSW);
	
	while(true)
	{
		// Block and wait until any input bit changed
		inputs = matrix->AwaitInputChange(-1);
		
		currA = checkPin(inputs, CLK_PIN);
		currSW = checkPin(inputs, SW_PIN);
		
		//fprintf(stderr,"Input change, lastSW = %d, currSW = %d\n",lastSW,currSW);
		//fprintf(stderr,"Input change, lastA = %d, currA = %d, currB = %d\n",lastA,currA,checkPin(inputs, DT_PIN));
		
		if (!lastA && currA) // rising edge of A - Input event
		{
			if (currA != checkPin(inputs, DT_PIN)) // B differs => CW rot
			{
				switch (moveDir)
				{
					case 0:
						if (xPos < 63)
							xPos++;
						break;
					case 1:
						if (yPos < 31)
							yPos++;
						break;
				}
			}
			
			else // CCW rot
			{
				switch (moveDir)
				{
					case 0:
						if (xPos > 0)
							xPos--;
						break;
					case 1:
						if (yPos > 0)
							yPos--;
						break;
				}
			}
		}
		
		if (!lastSW && currSW) // Button press event
		{
			if (moveDir == 0)
				moveDir = 1;
			else
				moveDir = 0;
		}
		
		lastSW = currSW;
		lastA = currA;
		
		refreshScreen = true;
	}
}

void drawLoop(RGBMatrix* &matrix, FrameCanvas* &offscreen)
{
	if (refreshScreen)
	{
		refreshScreen = false;
		
		offscreen->Clear();
		offscreen->SetPixel(xPos,yPos,255,0,255);
		//usleep(delay); // Set anim speed
		
		// Page flip - Tied to a fraction of the refresh rate
		offscreen = matrix->SwapOnVSync(offscreen, 1);
		
	}
}
