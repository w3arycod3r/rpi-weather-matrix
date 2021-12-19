/*
	Title: ppm-test.cc
	Author: Garrett Carter
	Date: 5/19/19
	Purpose: Test ppm class: read in a ppm file, display on matrix and move around
*/

#include "ppm.h"
#include <signal.h>
#include <unistd.h> // usleep

using namespace rgb_matrix;

//// GLOBAL VARS & FLAGS
bool interruptReceived = false;
std::string IMAGE_DIR = "./images/weather_icons/";
std::vector<std::string> ICONS = {"celsius","full-moon","half-moon","humidity","night","rain","rain-1","rainbow","snowflake","starry-night","storm","storm-1","sun","sunrise","thermometer","umbrella","wind"};
int NUM_ICONS = ICONS.size();

//// IMAGES VECTOR
std::vector<ppm*> images;

//// FUNCTION PROTOTYPES
static void InterruptHandler(int signo) { interruptReceived = true; }

int main(int argc, char** argv)
{
	ppm* newImage;
	//// LOAD IMAGES
	for (int i = 0; i < NUM_ICONS; i++)
	{
		newImage = new ppm(IMAGE_DIR + ICONS[i] + ".ppm");
		images.push_back(newImage);
	}
	
	// Create Matrix Object
	RGBMatrix::Options defaults;
	RuntimeOptions rtOps;
	
	defaults.rows = 32;
	defaults.cols = 64;
	defaults.brightness = 50;
	
	RGBMatrix* matrix = rgb_matrix::CreateMatrixFromOptions(defaults, rtOps);
	if (matrix == NULL)
		return 1;
	
	// Signal Handlers
	signal(SIGTERM, InterruptHandler);
	signal(SIGINT, InterruptHandler);

	int yPos = 0;
	int xPos = 1;
	//// Draw Images
	for (int i = 12; i < NUM_ICONS; i++)
	{
		images[i]->draw(matrix, xPos, yPos);
		xPos += 16;
		if ((i+1)%4 == 0)
		{
			xPos = 1;
			yPos += 16;
		}
	}
	
	while(!interruptReceived)
	{
		usleep(100);
	}
	
	
	matrix->Clear();
	delete matrix;
	
	for (int i = 0; i < NUM_ICONS; i++)
	{
		delete images[i];
	}
	
	fprintf(stderr,"\nBye! :)\n");
	return 0;
}
