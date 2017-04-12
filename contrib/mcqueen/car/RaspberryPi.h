/*
 * RaspberryPi.h
 *
 *  Created on: Mar 26, 2014
 *      Author: fabien
 */

#ifndef RASPBERRYPI_H_
#define RASPBERRYPI_H_

#include <string>
#include "Gpio.h"
#include "I2cBus.h"

#define 	CPUINFO			"/proc/cpuinfo"

using namespace std;

class RaspberryPi
{
public:
	RaspberryPi (void);
	~RaspberryPi (void);

	int init (void);
	int initI2cBus (void);
	int initGpio (void);

	int 	isReady (void);
	void 	printStatus(void);

	I2cBus	*getI2cBus (void);
	Gpio	*getGpio (void);
	Pin		*getPin (int pinNumber);

	int		getVersion (void);
	const char	*getModel (void);
	const char	*getSerial (void);

private:
	int 	ready;
	int 	cpuRevision;
	string	cpuModel;
	int 	version;
	string	cpuSerial;
	I2cBus	*i2c;
	Gpio	*gpio;

	void 	getCpuInfo (void);
	int 	getRevision (void);
};




#endif /* RASPBERRYPI_H_ */
