#include "Weather.h"

int main(int argc, char** argv)
{
    if (argc < 2)
        return 1;

    string filepath = argv[1];
    WeatherData* wd = new WeatherData(filepath);

    wd->printDebugData();

    delete wd;

    return 0;
}