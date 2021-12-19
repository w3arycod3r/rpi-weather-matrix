/*
	Title: text.cc
	Author: Garrett Carter
	Date: 5/4/19
	Purpose: Printing text on matrix, using rgbmatrix library
*/

#include "led-matrix.h"
#include "graphics.h"

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <iostream>

using namespace rgb_matrix;
using namespace std;

// Handle interrupt cleanly
volatile bool interrupt_received = false;
static void InterruptHandler(int signo) {
  interrupt_received = true;
}

int main(int argc, char** argv)
{
	RGBMatrix::Options ops; // Options struct
	RuntimeOptions rtOps; // RT Options struct
	
	// Matrix params
	ops.rows = 32;
	ops.cols = 64;
	ops.brightness = 50;
	
	RGBMatrix* matrix = CreateMatrixFromOptions(ops, rtOps);
	if (matrix == NULL) // Error creating matrix
	{
		cout << "Error creating matrix\n";
		return 1;
	}
	
	signal(SIGTERM, InterruptHandler); // Associate handler
	signal(SIGINT, InterruptHandler);
	
	// Load font
	Font font;
	if(!font.LoadFont("../rpi-rgb-led-matrix/fonts/5x7.bdf"))
	{
		cout << "Error loading font.\n";
		return 1;
	}
	
	Color text_color(255,0,255); // Purple
	int xPos = 3;
	int yPos = font.baseline();
	int vSpeed = 1; // Pixels per frame
	
	string text = "Hello World!";
	
	// Matrix is front buffer, offscreen is back buffer
	FrameCanvas* offscreen = matrix->CreateFrameCanvas();
	
	
	while(!interrupt_received)
	{
		offscreen->Clear();
		DrawText(offscreen, font, xPos, yPos, text_color, NULL, text.c_str(), 0);
		//usleep(delay); // Set anim speed
		
		// Page flip - Tied to a fraction of the refresh rate by argv[1]
		offscreen = matrix->SwapOnVSync(offscreen, atoi(argv[1]));
		
		// Bounce text vertically
		yPos += vSpeed;
		if (yPos == ops.rows || yPos == font.baseline()) 
			vSpeed = -vSpeed;
	}
	
	cout << endl;
	
	// Clear Matrix
	matrix->Clear();
	delete matrix;
	
	return 0;
}