/*
 * Gpio.h
 *
 *  Created on: Apr 1, 2014
 *      Author: fabien papleux
 *
 *  We will be using the GPIO class to control communications with individual
 *  IO pins.  The I2C, SPI and UART pins will be managed separately by their
 *  own classes.
 *
 */

#ifndef GPIO_H_
#define GPIO_H_

#include "Pin.h"

#define MODEL_RASPI1	1
#define MODEL_RASPI2	2

class Gpio
{

public:
	Gpio (int model);  // Use the RaspberryPi's "getVersion" method to feed the parameter of this constructor
	~Gpio();

	int		init		(void);
	int		isReady		(void);
	void	printStatus	(void);

	Pin		*getPin		(int physNumber); // Pins work based on their physical number

private:
	int		version;
	int		ready;
	Pin		*pin[27];	// holds pointers to Pin objects
};

#endif /* GPIO_H_ */
