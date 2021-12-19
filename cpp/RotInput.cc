/*
	Title: RotInput.cc
	Author: Garrett Carter
	Date: 5/14/19
	Purpose: RotInput Class - Interface for rotary encoder input & Optional pwr switch
*/

#include "RotInput.h"

// State Assignments
#define R_START     0x3
#define R_CW_BEGIN  0x1
#define R_CW_NEXT   0x0
#define R_CW_FINAL  0x2
#define R_CCW_BEGIN 0x6
#define R_CCW_NEXT  0x4
#define R_CCW_FINAL 0x5

// State Transition Table (Mealy type FSM)
const unsigned char ttable[8][4] = {
    {R_CW_NEXT,  R_CW_BEGIN,  R_CW_FINAL,  R_START},                // R_CW_NEXT
    {R_CW_NEXT,  R_CW_BEGIN,  R_CW_BEGIN,  R_START},                // R_CW_BEGIN
    {R_CW_NEXT,  R_CW_FINAL,  R_CW_FINAL,  R_START | DIR_CW},       // R_CW_FINAL
    {R_START,    R_CW_BEGIN,  R_CCW_BEGIN, R_START},                // R_START
    {R_CCW_NEXT, R_CCW_FINAL, R_CCW_BEGIN, R_START},                // R_CCW_NEXT
    {R_CCW_NEXT, R_CCW_FINAL, R_CCW_FINAL, R_START | DIR_CCW},      // R_CCW_FINAL
    {R_CCW_NEXT, R_CCW_BEGIN, R_CCW_BEGIN, R_START},                // R_CCW_BEGIN
    {R_START,    R_START,     R_START,     R_START}                 // ILLEGAL
};

RotInput::RotInput(pthread_t mainThread, int CLK_PIN, int DT_PIN, int SW_PIN, RGBMatrix* matrix,
				   int PWR_SW_PIN)
{
	this->mainThread = mainThread;
	this->CLK_PIN = CLK_PIN;
	this->DT_PIN = DT_PIN;
	this->SW_PIN = SW_PIN;
	this->PWR_SW_PIN = PWR_SW_PIN;
	this->matrix = matrix;
	state = R_START; // Initial State
	this->event = DIR_NONE;
	stateLock = PTHREAD_MUTEX_INITIALIZER;
	
	if (pthread_create(&inputThread, NULL, &processInputWrapper, this) != 0) // Start input proc thread
		fprintf(stderr,"Input Thread creation failed\n");
	else
		fprintf(stderr,"Input Thread Created\n");
}

void RotInput::processInput()
{
	// Init (reserve) all available inputs
	matrix->gpio()->RequestInputs(0xffffffff);
	
	usleep(1e5); // Delay to allow inputs to stabilize
	uint32_t inputs;
	
	bool pinA, pinB;
	bool lastPWR = true; 
	bool currPWR = true;
	bool lastSW = true;
	bool currSW;
	bool newEvent = false;
	
	unsigned char pinState;
	
	//fprintf(stderr,"A B\n");
	
	while(true)
	{
		// Block and wait until any input bit changed
		inputs = matrix->AwaitInputChange(-1);
		
		pinA = checkPin(inputs,CLK_PIN);
		pinB = checkPin(inputs,DT_PIN);
		currSW = checkPin(inputs,SW_PIN);
		newEvent = false; // Flag to indicate newEvent set => wakeup main thread to handle event
		
		if (PWR_SW_PIN != -1)
			currPWR = checkPin(inputs,PWR_SW_PIN);

		
		pinState = (pinA << 1) | pinB;
		// Determine next state from pins, curr state, and state table
		
		
		pthread_mutex_lock(&stateLock);
		
		state = ttable[state & 0x07][pinState];
		
		if (state & DIR_CW)
		{
			event = DIR_CW;
			newEvent = true;
		}
		if (state & DIR_CCW)
		{
			event = DIR_CCW;
			newEvent = true;
		}
		
		if (lastSW && !currSW)
		{
			event = SW_PRESS;
			newEvent = true;
		}

		if (lastPWR && !currPWR)
		{
			event = PWR_SW_PRESS;
			newEvent = true;
		}
		
		pthread_mutex_unlock(&stateLock);
		
		// Send signal to main thread, which will wake from any sleep to handle input event
		if (newEvent)
		{
			pthread_kill(mainThread, SIGUSR1);
		}

		if (PWR_SW_PIN != -1)
			lastPWR = currPWR;
		lastSW = currSW;
	}
}

RotInput::~RotInput()
{
	fprintf(stderr,"In RotInput destructor\n");
	pthread_cancel(inputThread); // Send cancellation request to thread
	pthread_join(inputThread, NULL); // Wait for thread to cancel
}