/*
 * Gpio.cpp
 *
 *  Created on: Apr 1, 2014
 *      Author: fabien papleux
 */
#include "SharkConfig.h"
#include <iostream>
#if ENABLE_WIRING_PI 
	#include <wiringPi.h>
#else
	#include "wiringPiLite.h"
#endif
#include "Pin.h"
#include "Gpio.h"


using namespace std;


/*
 * ---------------------------------------------------------------------------------------------------
 * CONSTRUCTOR
 * Must receive the Raspberry Pi revision number (1 or 2)
 * ---------------------------------------------------------------------------------------------------
 */
Gpio::Gpio (int raspiVersion)
{
	// Ensures that the object is recorded as not ready until full successful initialization
	ready = 0;
	version = raspiVersion;

	// Initializes all pins to null.  They will be initializes as part of the init sequence
	for (int t = 0; t <= 26; t++) pin[t] = 0;

	// Starts actual device initializing sequence
	init();
}




/*
 * ---------------------------------------------------------------------------------------------------
 * DESTRUCTOR
 * ---------------------------------------------------------------------------------------------------
 */
Gpio::~Gpio (void)
{
	ready = 0;
	// destroys all pin objects associated with this gpio
	for (int t = 0; t <= 26 ; t++)
	{
		if (pin[t]) delete pin[t];
	}
}

/*
 * ---------------------------------------------------------------------------------------------------
 * int init (void)
 *
 * ---------------------------------------------------------------------------------------------------
 */
int Gpio::init (void)
{
	// Standard pin set initialization
	pin[7] = new Pin(7);
	pin[11] = new Pin(11);
	pin[12] = new Pin(12);
	pin[13] = new Pin(13);
	pin[15] = new Pin(15);
	pin[16] = new Pin(16);
	pin[18] = new Pin(18);
	pin[22] = new Pin(22);
	pin[23] = new Pin(23);

	ready = 1;
	return ready;
}

int Gpio::isReady (void)
{
	return ready;
}

Pin	*Gpio::getPin(int number) {
	if ((number >= 0) && (number <= 26)) return pin[number];
	return 0;
}

void Gpio::printStatus (void)
{
	cout << "GPIO Status" << endl;
	cout << "-----------" << endl;
	cout << endl;
	cout << "Is Ready : " << (isReady() ? "Yes" : "No") << endl;
	for (int i = 1; i <= 26; i++) {
		if (pin[i] != 0) pin[i]->printStatus();
	}
	cout << endl;
}

