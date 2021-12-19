/*
	Title: ppm.cpp
	Author: Garrett Carter
	Date: 7/2/19
	Purpose: Contains function definitions for ppm and Frames classes
*/

#include "ppm.h"


//===============// PPM CLASS

void ppm::init() {
    width = 0;
    height = 0;
    max_col_val = 255;
}


ppm::ppm() {
    init();
}


ppm::ppm(const string &fname) {
    init();
    read(fname);
}


ppm::ppm(const unsigned int _width, const unsigned int _height) {
    init();
    width = _width;
    height = _height;

    size = width*height;

    // fill r, g and b with 0
    r.resize(size);
    g.resize(size);
    b.resize(size);
}


bool ppm::read(const string &fname)
{
    std::ifstream inp(fname.c_str(), std::ios::in | std::ios::binary);
    if (inp.is_open()) {
        string line;
        getline(inp, line);
        if (line != "P6") {
            cerr << "Error. Unrecognized file format." << endl;
            inp.close();
            return false;
        }
        getline(inp, line);
        while (line[0] == '#') {
            getline(inp, line); // Ignore comment lines
        }
        std::stringstream dimensions(line);

        try {
            dimensions >> width;
			if (!dimensions.eof())
			{
				dimensions >> height;
			}
			else // Handle case of width on next line
			{
				getline(inp, line);
				dimensions.str(line);
				dimensions >> height;
			}
        } catch (std::exception &e) {
            cerr << "Header file format error. " << e.what() << endl;
            inp.close();
            return false;
        }

		getline(inp, line); // Remove last newline before data
		std::stringstream max_col(line);
		
        try {
            max_col >> max_col_val;
        } catch (std::exception &e) {
            cerr << "Header file format error. " << e.what() << endl;
            inp.close();
            return false;
        }

        size = width*height;

        r.reserve(size);
        g.reserve(size);
        b.reserve(size);

        char aux;
        for (unsigned int i = 0; i < size; ++i) {
            inp.read(&aux, 1);
            r[i] = (unsigned char) aux;
            inp.read(&aux, 1);
            g[i] = (unsigned char) aux;
            inp.read(&aux, 1);
            b[i] = (unsigned char) aux;
        }
    } else {
        cerr << "Error. Unable to open " << fname << endl;
        inp.close();
        return false;
    }

    // Read operation was successful
    inp.close();
    return true;
}


bool ppm::write(const string &fname) {
    std::ofstream inp(fname.c_str(), std::ios::out | std::ios::binary);
    if (inp.is_open()) {

        inp << "P6\n"; // PPM format code
        inp << width;
        inp << " ";
        inp << height << "\n";
        inp << max_col_val << "\n";

        char aux;
        for (unsigned int i = 0; i < size; ++i) {
            aux = (char) r[i];
            inp.write(&aux, 1);
            aux = (char) g[i];
            inp.write(&aux, 1);
            aux = (char) b[i];
            inp.write(&aux, 1);
        }
    } else {
        cerr << "Error. Unable to open " << fname << endl;
        inp.close();
        return false;
    }
    // Success
    inp.close();
    return true;
}


void ppm::draw(Canvas* c, const int xPos, const int yPos)
{
	int i = 0; // Counter for vector indices
	for (int yOff = 0; yOff < height; yOff++)
	{
		for (int xOff = 0; xOff < width; xOff++)
		{
            if ( r[i]!=0 || g[i]!=0 || b[i]!=0 ) // Allow black pixels to be transparent
			    c->SetPixel(xPos+xOff, yPos+yOff, r[i],g[i],b[i]);
			i++;
		}
	}
}


void ppm::draw(Canvas* c, const double xPos, const double yPos)
{
	int _xPos = round(xPos);
    int _yPos = round(yPos);

    this->draw(c, _xPos, _yPos);
}


void ppm::drawCenter(Canvas* c, const int xPos, const int yPos)
{
    // Top left coords
    int xTL = xPos - round(width/2.0) + 1;  // +1 is a correction
    int yTL = yPos - round(height/2.0) + 1; // Ditto

    this->draw(c, xTL, yTL);
}


//==============// FRAMES CLASS


Frames::Frames()
{

}


Frames::Frames(const string &FILE_PATH, const int NUM_FRAMES)
{
    this->load(FILE_PATH, NUM_FRAMES);
}


Frames::~Frames()
{
    this->unload();
}


bool Frames::load(const string &FILE_PATH, const int NUM_FRAMES)
{
    bool allGood = true; // All frames loaded successfully
    unsigned int readCount = 0;

    for (int i = 0; i < NUM_FRAMES; i++)
	{
		string cmpFP = FILE_PATH + to_string(i) + ".ppm"; // Complete Filepath
		ppm* img = new ppm; // Create frame

        if (!img->read(cmpFP)) // Read in frame
            allGood = false;
        else
        {
            readCount++;
            images.push_back(img); // Add frame into vector
        }

	}
	cerr << readCount <<  " frames loaded from " << FILE_PATH << endl;

    return allGood;
}


void Frames::unload()
{
    const int SIZE = images.size();
	for (int i = 0; i < SIZE; i++)
	{
		delete images[i];
	}
    cerr << SIZE << " frames unloaded.\n";
}


void Frames::draw(const int INDEX, Canvas* c, const int XPOS, const int YPOS)
{
    images[INDEX]->draw(c, XPOS, YPOS);
}

void Frames::drawCenter(const int INDEX, Canvas* c, const int XPOS, const int YPOS)
{
    images[INDEX]->drawCenter(c, XPOS, YPOS);
}