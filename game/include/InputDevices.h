/*
	Title: InputDevices.h
	Author: Garrett Carter
	Date: 9/30/19
	Purpose: Classes for working with joystick and keyboard input systems in linux.
			 Includes constants for working with PS3 controllers specifically.
*/


#ifndef INPUT_DEVICES_H
#define INPUT_DEVICES_H

#include <string>
#include <cstring> // memset()
#include <iostream>
#include <iomanip>
#include <linux/input.h>
#include <linux/joystick.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sstream>
#include "unistd.h"
#include <queue>
#include <cmath> // round()

using std::endl; using std::cerr; using std::setw;
using std::queue;
using std::string;

#define JS_EVENT_BUTTON 0x01 // button pressed/released
#define JS_EVENT_AXIS   0x02 // joystick moved
#define JS_EVENT_INIT   0x80 // initial state of device

// PS3 Controller Button Codes

#define PS3_BTN_SELECT		0  // Select Button
#define PS3_BTN_L3			1  // Left Joystick Click
#define PS3_BTN_R3			2  // Right Joystick Click
#define PS3_BTN_START		3
#define PS3_BTN_UP			4
#define PS3_BTN_RIGHT		5
#define PS3_BTN_DOWN		6
#define PS3_BTN_LEFT		7
#define PS3_BTN_L2			8  // Left Trigger
#define PS3_BTN_R2			9  // Right Trigger
#define PS3_BTN_L1			10 // Left Bumper
#define PS3_BTN_R1			11 // Right Bumper
#define PS3_BTN_TRIANGLE	12
#define PS3_BTN_CIRCLE		13
#define PS3_BTN_X			14
#define PS3_BTN_SQUARE		15
#define PS3_BTN_PS			16 // PlayStation Button

// PS3 Controller Axis Numbers

#define PS3_LEFT_STICK_X	0
#define PS3_LEFT_STICK_Y	1
#define PS3_RIGHT_STICK_X	2
#define PS3_RIGHT_STICK_Y	3

// Joystick Axis Range
#define MIN_AXES_VALUE 		-32768
#define MAX_AXES_VALUE  	32767

/************
 Device Types
************/
// Keyboard type opens just event file
#define DEV_KEYBOARD	1
// Joystick type opens just js file
#define DEV_JOYSTICK	2
// PS3 controller type must open both the js and event files, to enable rumble support
#define DEV_PS3			3

/***********
 Event Types
***********/
#define EVT_KEYBOARD	1
#define EVT_JOYSTICK	2

/* 
 * Encapsulates all info relevant to a sampled keyboard or joystick event.
 */
class InputEvent
{
private:
	// type: See EVT_ event types
	uint8_t type;
public:
	struct input_event ie; // Populated for Keyboard events
	struct js_event jse;   // Populated for Joystick events

	bool isKeyboardEvent() const { return (type == EVT_KEYBOARD); }
	bool isJoystickEvent() const { return (type == EVT_JOYSTICK); }

	// Joystick event filters
	bool isButton() const;
	bool isButtonDown() const { return (isButton() && jse.value == 1); }
	bool isButtonDown(uint8_t number) const
	{
		return (isButtonDown() && jse.number == number);
	}
	bool isButtonUp() const { return (isButton() && jse.value == 0); }
	bool isButtonUp(uint8_t number) const
	{
		return (isButtonUp() && jse.number == number);
	}
	bool isInitState() const;

	// Keyboard event filters
	bool isKey() const;
	bool isKeyDown() const { return (isKey() && ie.value == 1); }
	bool isKeyUp() const { return (isKey() && ie.value == 0); }
	bool isAutoRepeat() const;
	
	// Filters for both types of events
	bool isAxis() const;
	
	friend class InputDevice;
};

/**
 * Stream insertion function so you can do this:
 *    cout << event << endl;
 */
std::ostream& operator<<(std::ostream& os, const InputEvent& e);


/**
 * Represents a keyboard or a joystick device. Allows data to be sampled from it.
 * Uses non-blocking read.
 */
class InputDevice
{
private:
	int event_fd = -1;
	int js_fd = -1;

	string event_fp = "";
	string js_fp = "";
	string macAddress  = "";

	// DEV_KEYBOARD, DEV_JOYSTICK, or DEV_PS3
	uint8_t typeOfDevice = 0;

	// Opens with little error checking
	void openRawPath(string devicePath, uint8_t typeOfDevice);

public:
	/* Attempt to open specified device. 
	 * If a device is already attached and found, these will do nothing.
	 * typeofDevice: Should be DEV_KEYBOARD or DEV_JOYSTICK
	 * openPath() is used for a basic keyboard or a joystick when no rumble support is desired. Use when you know exact path by-id.
	 * openPS3() assumes a special udev rule is setup for PS3 Controllers. This opens the js and event files. Supports rumble.
	 * openJoystick() or openKeyboard() is used when you know the assigned device #.
	 */ 
	void openPath(string devicePath, uint8_t typeOfDevice);
	void openPS3(string macAddress);
	void openJoystick(int joystickNumber);
	void openKeyboard(int keyboardNumber);

	// For Keyboards, send ioctl to disable auto repeat key events. This is accomplished by setting REP_DELAY to 0.
	void disableAutoRepeat();

	// Reattempt a connection. An open() method must be called first, to set the filepath(s).
	void retryConnection();

	/* Returns 0 if no device is open. If a device is open, the type is returned.
	 */
	uint8_t getTypeOpen();

	/* Setup Rumble effect in PS3 controller's memory.
	 * A PS3 controller has both strong and weak motors. 
	 * Motor intensities should be 0->100 %, inclusive. Duration is in msec (max 32767).
	 * Returns the id assigned to the effect.
	 * Seems to require a disconnect and reconnect to change effects.
	 */
	int setupEffect(int strongIntensity, int weakIntensity, int duration);
	/* Play the effect with the specified id.
	 * 
	 */
	void playEffect(int id);
	/* Remove effect with the specified id from the device memory, freeing space for more.
	 * Also stops the effect if it was playing.
	 */
	void removeEffect(int id);
	/* Stop the effect specified, if it is currently playing.
	 * Seems to stop any effects playing, if you pass a valid id which is present in controller.
	 */
	void stopEffect(int id);


	// Closes file descriptor
	~InputDevice()
	{
		closeFile();
	}
	void closeFile()
	{
		close(event_fd); // System call
		close(js_fd);

		typeOfDevice = 0;
		event_fd = -1;
		js_fd = -1;
	}

	// Must initialize (open) the device before using.
	InputDevice()
	{

	}
	/**
	 * InputDevice objects cannot be copied
	 */
	InputDevice(InputDevice const&) = delete;

	/**
	 * InputDevice objects can be moved
	 */
	InputDevice(InputDevice &&) = default;

	bool jsFileOpen() { return js_fd >= 0; }

	bool eventFileOpen() { return event_fd >= 0; }

	/**
	 * Returns true if a device is properly attached and and may be used, otherwise false.
	 */
	bool isFound()
	{
		if (typeOfDevice == DEV_KEYBOARD)
			return eventFileOpen();

		if (typeOfDevice == DEV_JOYSTICK)
			return jsFileOpen();

		if (typeOfDevice == DEV_PS3)
			return eventFileOpen() && jsFileOpen();
		
		return false;
	}
	
	/**
	 * Attempts to populate the provided InputEvent instance with data
	 * from the device. In the case of the PS3 controller, this will use the js file.
	 * Returns: 0 = No data available, 1 = Some Fatal Error, 2 = Good Read
	 */
	uint8_t sample(InputEvent* event);
};


#endif
