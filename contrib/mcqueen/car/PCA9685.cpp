/*
 * PCA9685.cpp
 *
 * Implements the PCA9685 PWM controller
 *
 *  Created on: Mar 24, 2014
 *      Author: fabien papleux
 */

#include <iostream>
#include "I2cBus.h"
#include "PCA9685.h"
#include <stdio.h>

using namespace std;

/********************************************************/
PCA9685::PCA9685 (void)
{
	init(NULL, DEFAULT_PCA9685_ADDRESS, DEFAULT_PCA9685_FREQUENCY, RESOLUTION, OSC_CLOCK);
}

/********************************************************/
PCA9685::PCA9685 (I2cBus *i2cBus)
{
	init(i2cBus, DEFAULT_PCA9685_ADDRESS, DEFAULT_PCA9685_FREQUENCY, RESOLUTION, OSC_CLOCK);
}

/********************************************************/
PCA9685::PCA9685 (I2cBus *i2cBus, int addr, int freq)
{
	init(i2cBus, addr, freq, RESOLUTION, OSC_CLOCK);
}

/********************************************************/
PCA9685::~PCA9685(void)
{
}

/********************************************************/
void PCA9685::reset (void)
{
	setAllOff();
	// Insert here code to software reset the controller
}

/********************************************************/
void PCA9685::printStatus (void)
{
	cout << "PCA9685 PWM CONTROLLER STATUS" << endl;
	cout << "-----------------------------" << endl;
	cout << "Is Ready           : " << (isReady() ? "Yes" : "No") << endl;
	cout << "Is asleep          : " << (isAsleep() ? "Yes" : "No") << endl;
	cout << "Address on I2C Bus : 0x" << hex << address << dec << endl;
	cout << "PWM frequency      : " << getFrequency() << "Hz" << endl;
	cout << "Mode1 Register     : 0x" << hex << i2c->read8(address, MODE1_REG) << dec << endl;
	cout << endl;
}

/********************************************************/
void PCA9685::sleep (void)
{
	if (! i2c) return;
	i2c->write8(address, MODE1_REG, MODE1_SLEEP);
}

/********************************************************/
void PCA9685::wakeUp (void)
{
	if (! i2c) return;
	i2c->write8(address, MODE1_REG, MODE1_AI);
}

/********************************************************/
void PCA9685::setAllOff ()
{
	if (! i2c) return;
	i2c->write8(address, LED_ALL_ON, 0);
	i2c->write8(address, LED_ALL_ON + 1, 0);
	i2c->write8(address, LED_ALL_ON + 2, 0);
	i2c->write8(address, LED_ALL_ON + 3, 0);
}



/********************************************************/
int PCA9685::isReady (void)
{
	return ready;
}

/********************************************************/
int PCA9685::isAsleep (void)
{
	if (! i2c) return -1;
	return (i2c->read8(address, MODE1_REG) & MODE1_SLEEP);
}

/********************************************************/
int PCA9685::getPwm (int channel)
{
	if (! i2c) return -1;
	channel = (channel * 4) + 6;
	int regLo = i2c->read8 (address, channel + 2);
	int regHi = i2c->read8 (address, channel + 3);
	return (regHi << 8) + regLo;
}

/********************************************************/
int PCA9685::getAddress (void)
{
	return address;
}

/********************************************************/
int PCA9685::getFrequency (void)
{
	if (! i2c) return -1;
	int scale = i2c->read8 (address, PRESCALE_REG);

	//avoid divide by zero when PCA9685 is absent or not working.	
	if(scale == -1)
		return -1;

	return (25000000 / (scale + 1)) / 4096;
}

/********************************************************/
I2cBus *PCA9685::getI2cBus (void)
{
	return i2c;
}

/********************************************************/
int PCA9685::getResolution (void)
{
	return resolution;
}

/********************************************************/
int PCA9685::setPwm (int channel, int data)
{
	if (! i2c) return -1;
	int regLo = data & 0x00ff;
	int regHi = data >> 8;
	int reg = (channel * 4) + 6;
	//cout << "Setting PWM for channel " << channel << " registers starting 0x" << hex << reg << " total value of 0x" << data << " in 2 bytes: 0x" << regHi << " & 0x" << regLo << dec << endl;
	i2c->write8 (address, reg, 0x00);
	i2c->write8 (address, reg + 1, 0x00);
	i2c->write8 (address, reg + 2, regLo);
	i2c->write8 (address, reg + 3, regHi);
	return 1;
}

/********************************************************/
void PCA9685::setAddress (int addr)
{
	setAllOff();
	address = addr;
	reset();
}

/********************************************************/
void PCA9685::setFrequency (int freq)
{
	if (! i2c) return;
	int reg = (25000000 / (4096 * freq)) - 1;
	sleep();
	i2c->write8 (address, PRESCALE_REG, reg);
	wakeUp();
}

/********************************************************/
void PCA9685::setI2cBus (I2cBus *i2cBus)
{
	setAllOff();
	i2c = i2cBus;
	reset();
}





/********************************************************/
int PCA9685::init (I2cBus *i2cBus, int addr, int freq, int res, int clock)
{
	ready = 0;
	i2c = i2cBus;
	address = addr;
	resolution = res;
	oscClock = clock;
	setFrequency(freq);
	if (i2c) ready = 1;

	if(getFrequency() == -1)
		ready = 0;

	// insert here any additional test required to establish that the controller is ready
	return ready;
}
