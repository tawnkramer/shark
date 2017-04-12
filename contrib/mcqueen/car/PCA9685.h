/*
 * PwmController.h
 *
 *  Created on: Mar 24, 2014
 *      Author: fabien papleux
 *
 *  Library made for an I2C PWM Controller
 */

#ifndef PCA9685_H_
#define PCA9685_H_

#include "I2cBus.h"

#define OSC_CLOCK 			25000000
#define RESOLUTION			4096
#define PRESCALE_REG		0xfe
#define LED_ALL_ON			0xfa

#define DEFAULT_PCA9685_ADDRESS		0x40
#define DEFAULT_PCA9685_FREQUENCY	50		// Hz

#define MODE1_REG			0x00
#define MODE1_RESTART		0x80
#define MODE1_EXTCLK		0x40
#define MODE1_AI			0x20
#define MODE1_SLEEP			0x10
#define MODE1_SUB1			0x08
#define MODE1_SUB2			0x04
#define MODE1_SUB3			0x02
#define MODE1_ALLCALL		0x01


class PCA9685
{
public:
	PCA9685 (void);
	PCA9685 (I2cBus *i2cBus);
	PCA9685 (I2cBus *i2cBus, int addr, int freq);
	~PCA9685 (void);

	void reset(void);
	void printStatus (void);						// Sends detailed status of the controller to the screen
	void sleep ();								// Turns off all output and puts the controller in sleep mode
	void wakeUp ();								// Takes controller out of sleep mode and activate auto increment
	void setAllOff ();							// Turns off all output

	int isReady (void);							// returns 1 if controller is ready
	int isAsleep (void);							// returns 1 if the controller is set to sleep mode

	int getPwm (int channel);					// Gets the stop value of the PWM
	int getAddress (void);						// returns the address of the controller on the i2cBus
	int getFrequency (void);					// Returns the current frequency
	I2cBus *getI2cBus (void);					// Returns a pointer to the i2cBus object in use
	int getResolution (void);					// Returns the resolution of the PWM (12-bit for the PCA9685, which is 4096)

	int setPwm (int channel, int data);		// Sets the start & stop PWM value for me (still figure out meRef)
	void setAddress (int address);
	void setFrequency (int frequency);
	void setI2cBus (I2cBus *i2c);

private:
	int init (I2cBus *i2cBus, int addr, int res, int freq, int clock); 	// initializes the controller
	int ready;
	I2cBus *i2c;
	int address;
	int resolution;
	int oscClock;
};




#endif /* PCA9685_H_ */
