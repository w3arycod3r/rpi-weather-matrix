#include "InputDevices.h"



// << Overload for InputEvent
std::ostream& operator<<(std::ostream& os, const InputEvent& e)
{
	int spacing = 8;
	if (e.isKeyboardEvent())
	{
		os  << std::left
		    << "Keyboard: type=" 
			<< setw(spacing) << static_cast<int>(e.ie.type)
			<< " code="
			<< setw(spacing) << static_cast<int>(e.ie.code)
			<< " value="
			<< setw(spacing) << static_cast<int>(e.ie.value);
	}
	if (e.isJoystickEvent())
	{
		os  << std::left
		    << "Joystick: value=" 
			<< setw(spacing) << static_cast<int>(e.jse.value)
			<< " type="
			<< setw(spacing) << static_cast<int>(e.jse.type)
			<< " number="
			<< setw(spacing) << static_cast<int>(e.jse.number);
	}
	return os;
}

/****************************************
 InputEvent Indicator/Filtering Functions
****************************************/

bool InputEvent::isButton() const
{
	if (isJoystickEvent() && ((jse.type & JS_EVENT_BUTTON) != 0))
		return true;
	else
		return false;
}

bool InputEvent::isKey() const
{
	if (isKeyboardEvent() && ie.type == EV_KEY)
		return true;
	else
		return false;
}

bool InputEvent::isAutoRepeat() const
{
	if (ie.value == 2)
		return true;
	else
		return false;
}
bool InputEvent::isAxis() const
{
	if (isKeyboardEvent() && ((ie.type == EV_ABS) || (ie.type == EV_REL)))
		return true;
	if (isJoystickEvent() && ((jse.type & JS_EVENT_AXIS) != 0))
		return true;
	
	return false;
}
bool InputEvent::isInitState() const
{
	if (isKeyboardEvent())
		return false;
	if (isJoystickEvent() && ((jse.type & JS_EVENT_INIT) != 0))
		return true;

	return false;
}

/*********************
 InputDevice Functions
*********************/

void InputDevice::openRawPath(string devicePath, uint8_t typeOfDevice)
{
	// Assume type is valid and assign identity
	this->typeOfDevice = typeOfDevice;
	// Open the device using non-blocking
	if (typeOfDevice == DEV_KEYBOARD)
	{
		event_fd = open(devicePath.c_str(), O_RDWR | O_NONBLOCK);
		event_fp = devicePath;
	}
	if (typeOfDevice == DEV_JOYSTICK)
	{
		js_fd = open(devicePath.c_str(), O_RDWR | O_NONBLOCK);
		js_fp = devicePath;
	}
}

void InputDevice::openPath(string devicePath, uint8_t typeOfDevice)
{
	if (isFound())
		return;
	if (typeOfDevice > DEV_PS3) // Error checking
		return;

	openRawPath(devicePath, typeOfDevice);
}

void InputDevice::openPS3(string macAddress)
{
	std::stringstream event_fp, js_fp;

	event_fp << "/dev/input/by-id/PLAYSTATION_R_3_Controller__" << macAddress << "_event";
	js_fp << "/dev/input/by-id/PLAYSTATION_R_3_Controller__" << macAddress << "_js";

	// Open files if they are not already open
	if (!eventFileOpen())
		openRawPath(event_fp.str(), DEV_KEYBOARD);
	if (!jsFileOpen())
		openRawPath(js_fp.str(), DEV_JOYSTICK);

	this->typeOfDevice = DEV_PS3;
	this->macAddress = macAddress;
}

int InputDevice::setupEffect(int strongIntensity, int weakIntensity, int duration)
{
	struct ff_effect ffe;

	// Setup parameters
	memset(&ffe, 0, sizeof(ffe));
	
	ffe.type = FF_RUMBLE;
	ffe.id = -1;

	ffe.replay.length = duration;
	ffe.replay.delay = 0; // No effect with sixad

	// 0x8000 = strong max, 0xc000 = weak max?
	ffe.u.rumble.strong_magnitude = round(strongIntensity/100.0 * 0x8000);
	ffe.u.rumble.weak_magnitude = round(weakIntensity/100.0 * 0xc000);

	// System call to upload effect
	if (ioctl(event_fd, EVIOCSFF, &ffe) == -1)
		perror("Upload effect");
	else
		cerr << "Upload Success, id: " << ffe.id << endl;

	return ffe.id;
}

void InputDevice::playEffect(int id)
{
	// Setup play command
	struct input_event play;
	memset(&play, 0, sizeof(play));

	play.type = EV_FF;
	play.code = id;
	play.value = 1; // Play count - No effect with sixad

	// System call to play effect
	if (write(event_fd, (const void*) &play, sizeof(play)) == -1)
		perror("Play effect");
}

void InputDevice::removeEffect(int id)
{
	// Remove previously uploaded effect, if present. This also stops the effect, if it was playing
	if (id != -1)
	{
		if (ioctl(event_fd, EVIOCRMFF, id) == -1)
			perror("Remove effect"); // Print error based on Linux errno 
	}
}

void InputDevice::stopEffect(int id)
{
	// Setup stop command
	struct input_event stop;
	memset(&stop, 0, sizeof(stop));

	stop.type = EV_FF;
	stop.code = id;
	stop.value = 0;

	// System call to play effect
	if (write(event_fd, (const void*) &stop, sizeof(stop)) == -1)
		perror("Stop effect");
}


void InputDevice::openJoystick(int joystickNumber)
{
	std::stringstream sstm;
	sstm << "/dev/input/js" << joystickNumber;
	openPath(sstm.str(), DEV_JOYSTICK);
}
void InputDevice::openKeyboard(int keyboardNumber)
{
	std::stringstream sstm;
	sstm << "/dev/input/event" << keyboardNumber;
	openPath(sstm.str(), DEV_KEYBOARD);
}

void InputDevice::disableAutoRepeat()
{
	if (!isFound() || typeOfDevice != DEV_KEYBOARD)
		return;

	// Read settings
	unsigned int repeatSettings[2];
	ioctl(event_fd, EVIOCGREP, repeatSettings);

	// Write settings
	repeatSettings[0] = 0; // REP_DELAY
	ioctl(event_fd, EVIOCSREP, repeatSettings);

	//cerr << "REP_DELAY = " << repeatSettings[0];
	//cerr << "\nREP_PERIOD = " << repeatSettings[1] << endl;

	
}

void InputDevice::retryConnection()
{
	if (isFound())
		return;

	// Assuming these file paths have already been set
	if (typeOfDevice == DEV_KEYBOARD)
		openRawPath(event_fp, DEV_KEYBOARD);

	if (typeOfDevice == DEV_JOYSTICK)
		openRawPath(js_fp, DEV_JOYSTICK);

	if (typeOfDevice == DEV_PS3)
		openPS3(macAddress);
}

/* 
 * Returns 0 if no device is open. If a device is open, the type is returned.
 */
uint8_t InputDevice::getTypeOpen()
{
	if (isFound())
		return typeOfDevice;
	else
		return 0;
}

uint8_t InputDevice::sample(InputEvent* event)
{
	if (!isFound())
		return 1;

	int bytes = -1;
	if (typeOfDevice == DEV_KEYBOARD)
	{
		bytes = read(event_fd, &(event->ie), sizeof(event->ie));
		event->type = EVT_KEYBOARD;
	}
	if (typeOfDevice == DEV_JOYSTICK || typeOfDevice == DEV_PS3)
	{
		bytes = read(js_fd, &(event->jse), sizeof(event->jse));
		event->type = EVT_JOYSTICK;
	}
	
	int err = errno; // Sample errno immediately

	// This means no data was read
	if (bytes == -1)
	{
		if (err == EAGAIN) // No data was available
			return 0;
		else // Some other error caused no data to be read
			return 1;
	}

	// Expected number of bytes were read
	if ( (bytes == sizeof(event->ie)) && (typeOfDevice == DEV_KEYBOARD) )
		return 2;
	if ( (bytes == sizeof(event->jse)) && (typeOfDevice == DEV_JOYSTICK || typeOfDevice == DEV_PS3) )
		return 2;
	else // Size mismatch means something went wrong
		return 1;
}






