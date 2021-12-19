/*
	Title: ppm.h
	Author: Garrett Carter
	Date: 7/2/19
	Contents:   ppm class - Process and display a binary ppm file (P6 format)
                Frames class - Container class for ppm objects
*/


#ifndef PPM_H
#define PPM_H

#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <exception>
#include <cmath>

#include "led-matrix.h"
#include "graphics.h"

using namespace rgb_matrix; using std::string; using std::vector; using std::getline; using std::to_string;
using std::cerr; using std::endl;



//==============// PPM CLASS
class ppm {
	
private:
    // init(): Initializes attributes to default values.
    void init();

public:

    //=============// ATTRIBUTES

    // Vectors for storing the R,G,B values
    vector<unsigned char> r;
    vector<unsigned char> g;
    vector<unsigned char> b;

    // Height and width of the image
    unsigned int height;
    unsigned int width;

    // max color value (defines color resolution)
    unsigned int max_col_val;

    // total number of elements (pixels)
    unsigned int size;

    //=============// FUNCTIONS

    // ppm(): Basic constructor, Calls init().
    ppm();

    // ppm(fname): Creates a PPM object and fill it with data stored in fname using read().
    ppm(const string &fname);

    // ppm(width, height): Creates an "empty" PPM image with a given width and height; the R,G,B arrays are filled with zeros.
    ppm(const unsigned int _width, const unsigned int _height);

    // read(): Read the PPM image from fname into memory. Returns false and writes to cerr upon error.
    bool read(const string &fname);

    // write(): Write the PPM image from memory into fname using ppm (P6) image format. Returns false and writes to cerr upon error.
    bool write(const string &fname);
	

    // draw(Canvas*, int, int): Draws the image on the canvas specified. The coordinates are the TOP-LEFT of the image
	void draw(Canvas* c, const int xPos, const int yPos);

    // draw(Canvas*, double, double): Rounds the doubles into ints and calls draw() with ints.
	void draw(Canvas* c, const double xPos, const double yPos);

    // drawCenter(): Draws the image on the canvas specified. The coordinates are the approximate CENTER of the image
    void drawCenter(Canvas* c, const int xPos, const int yPos);

    
};


//==============// FRAMES CLASS
class Frames
{

private:
    // images: Holds the pointers to DMA ppm objects held in memory
    vector<ppm*> images;


public:
    // Getters
    int getNumFrames() { return images.size(); }
    int getImgWidth(int i) { return images[i]->width; }
    int getImgHeight(int i) { return images[i]->height; }


    // Frames(): Empty constructor
    Frames();

    // Frames(FILE_PATH, NUM_FRAMES): Calls load() using the parameters given
    Frames(const string &FILE_PATH, const int NUM_FRAMES);

    // ~Frames(): Calls unload()
    ~Frames();

    /*
	Function: load()
	Purpose: Loads in ppm images referenced by the filepath (see below example) into the "images" vector.
		Make sure the destructor gets called to release the DMA memory.

	Example filepath: "/images/Wonder" and numFrames = 5 will expand to:
		"images/Wonder0", "images/Wonder1", "images/Wonder2", "images/Wonder3", "images/Wonder4"
    */
    bool load(const string &FILE_PATH, const int NUM_FRAMES);

    // unload(): Frees the memory held by the DMA ppm images held in the "images" vector
    void unload();

    // draw(): Calls the draw() function for the appropriate ppm image held in the vector, using the parameters given
    void draw(const int INDEX, Canvas* c, const int XPOS, const int YPOS);

    // drawCenter(): Draws the indexed image on the canvas specified. The coordinates are the approximate CENTER of the image
    void drawCenter(const int INDEX, Canvas* c, const int XPOS, const int YPOS);
};

#endif