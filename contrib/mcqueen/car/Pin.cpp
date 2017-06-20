/* Pin.cpp
 *
 * created on 4/2/2014
 * author: fabien papleux
 *
 */
#include "SharkConfig.h"
#include <iostream>
#include <string>
#include <cstring>
#if ENABLE_WIRING_PI 
#include <wiringPi.h>
#include "Pin.h"

using namespace std;

// Mapping from physical pin number to wiringPi number
static int toWiringPi[27] =
{
		-1,	-1,	-1,	8,	-1,	9,	-1,	7,
		15,	-1,	16,	0,	1,	2,	-1,	3,
		4,	-1,	5,	12,	-1,	13,	6,	14,
		-1,	11
};


Pin::Pin (int newNumber)
{
	// Assumes 'wiringPiSetup' is called only once, and not at Pin level
	ready = 0;
	pinNumber = newNumber;
	wpiNumber = toWiringPi[newNumber];	// translates into wiringPi numbering for calls
	init();
}

Pin::~Pin (void)
{
}

int Pin::init (void)
{
	ready = 0;
	mode = OUTPUT;
	value = LOW;
	pinMode (wpiNumber, mode);
	digitalWrite (wpiNumber, LOW);
	ready = 1;
	// cout << "Pin " << pinNumber << " ready" << endl;
	return ready;
}

int Pin::isReady (void)
{
	return ready;
}

void Pin::printStatus (void)
{
	cout << "Pin " << pinNumber << " (Phys), " << wpiNumber << " (wpi): " << "MODE = " << mode << ", VALUE = " << value << endl;
}


int Pin::setMode (int newMode)
{
	if ((newMode < 0) || (newMode > 1)) return 0;
	pinMode (wpiNumber, newMode);
	mode = newMode;
	return 1;
}


int Pin::getMode (void)
{
	return mode;
}


int Pin::setValue (int newValue)
{
	digitalWrite (wpiNumber, newValue);
	value = newValue;
	return 1;
}


int Pin::getValue (void)
{
	if (mode == INPUT) {
		value = digitalRead (wpiNumber);
	}
	return value;
}


#endif // ENABLE_WIRING_PI 

