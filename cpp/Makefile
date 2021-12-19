# Makefile for custom code using the rgbmatrix library
# Library files are stored at ../rpi-rgb-led-matrix

# LED Matrix Library by hzeller
LED = ../rpi-rgb-led-matrix

# Header and Library flags for compilation
INC = -I$(LED)/include/ -I./
LIB = -pthread -L$(LED)/lib/ -lrgbmatrix


# Targets
all: exec text rot-en weather-disp rot-test ppm-test
main: weather-disp
clean:
	rm *.o exec text rot-en weather-disp rot-test ppm-test


# Link files and libs
exec:	minimal-example.o
	g++ -O3 -o exec minimal-example.o $(LIB)
	
text:	text.o
	g++ -O3 -o text text.o $(LIB)
	
rot-en: rot-en.o
	g++ -O3 -o rot-en rot-en.o $(LIB)
	
weather-disp: weather-disp.o Weather.o RotInput.o ppm.o
	g++ -O3 -o weather-disp weather-disp.o Weather.o RotInput.o ppm.o $(LIB)
	
rot-test: rot-test.o RotInput.o
	g++ -O3 -o rot-test rot-test.o RotInput.o $(LIB)
	
ppm-test: ppm-test.o ppm.o
	g++ -O3 -o ppm-test ppm-test.o ppm.o $(LIB)

# Compile into .o files
minimal-example.o:	minimal-example.cc
	g++ -O3  $(INC) -c minimal-example.cc
	
text.o: text.cc
	g++ -O3 $(INC) -c text.cc
	
rot-en.o: rot-en.cc
	g++ -O3 $(INC) -c rot-en.cc
	
weather-disp.o: weather-disp.cc Weather.h RotInput.h ppm.h weather_config.h
	g++ -O3 $(INC) -c weather-disp.cc
	
Weather.o: Weather.h Weather.cc
	g++ -O3 $(INC) -c Weather.cc

RotInput.o: RotInput.h RotInput.cc
	g++ -O3 $(INC) -c RotInput.cc
	
rot-test.o: rot-test.cc RotInput.h
	g++ -O3 $(INC) -c rot-test.cc
	
ppm-test.o: ppm-test.cc ppm.h
	g++ -O3 $(INC) -c ppm-test.cc
	
ppm.o: ppm.cpp ppm.h
	g++ -O3 $(INC) -c ppm.cpp