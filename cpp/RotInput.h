/*
	Title: RotInput.h
	Author: Garrett Carter
	Date: 5/14/19
	Purpose: RotInput Class - Interface for rotary encoder input & Optional pwr switch
*/
#ifndef ROT_INPUT_H
#define ROT_INPUT_H

#include "led-matrix.h"
#include <pthread.h>
#include <signal.h>
#include <unistd.h>
#include <stdio.h>

using namespace rgb_matrix;

// Values returned by getEvent()
#define DIR_NONE 0x00
#define DIR_CW 0x10
#define DIR_CCW 0x20
#define PWR_SW_PRESS 0x30
#define SW_PRESS 0x40

class RotInput
{
	public:
		/*
			Constructor:
			Params:
			Send mainThread (thread ID) from main using pthread_self()
			PWR_SW_PIN is optional

			The input thread will send SIGUSR1 to mainThread when a new input is recieved, to wakeup
			any sleeping in mainThread
			You need to setup a basic signal handler for SIGUSR1 so it is not interpreted as a terminate
		*/
		RotInput(pthread_t mainThread, int CLK_PIN, int DT_PIN, int SW_PIN, RGBMatrix* matrix,
				 int PWR_SW_PIN = -1);
		/* 
			Destructor:
			
		*/
		~RotInput();
		
		unsigned char getEvent()
		{
			// Implement a queue (FIFO) for input events
			
			pthread_mutex_lock(&stateLock);
			
			unsigned char _event = event;
			
			if (event != DIR_NONE)
				event = DIR_NONE; // Event handled

			pthread_mutex_unlock(&stateLock);
			return _event;
		}
		
	private:
		int CLK_PIN, DT_PIN, SW_PIN, PWR_SW_PIN;
		RGBMatrix* matrix;
		
		pthread_mutex_t stateLock;
		
		void processInput();
		// A static wrapper is needed for thread creation
		static void* processInputWrapper(void* object)
		{
			reinterpret_cast<RotInput*>(object)->processInput();
			return 0;
		}
		
		pthread_t inputThread;
		pthread_t mainThread;
		
		bool checkPin(uint32_t inputs, int pin)
		{
			return inputs & (1 << (pin)); // Extract bit #(pin) from the right
		}
		
		unsigned char state; // For State machine
		unsigned char event; // Holds current input event
};

#endif // ROT_INPUT_H