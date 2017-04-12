/*
 * PwmServo.cpp
 *
 *  Created on: Mar 23, 2014
 *      Author: fabien papleux
 */

#include <iostream>
#include <stdio.h>
#include <cmath>
#include "PCA9685.h"
#include "PwmServo.h"

using namespace std;



/**
 * -----------------------------------------------------------------------------------------------------
 * CONSTRUCTOR
 * initializes the Servo (if the PWM controller has already been initialized)
 * Receives:
 * 		-	A PwmServoConfig* that should contain the user-set values to determine the PWM values
 * 			to send the controller to achieve various tasks.  That configuration will be saved as
 * 			the base configuration.  PwmServoConfig is a struct defined in PwmServo.h.
 *
 *			The base configuration needs to be adjusted based on the actual PWM controller
 * 			configuration because PWM controllers can take different forms:  At the end of the day
 * 			a typical Servo will be on neutral with a 1.5ms pulse, full reverse with a 1ms pulse and
 * 			full forward with a 2ms pulse. (Same for most servos)
 *
 * 			The PCA9685 controller that I use can be set to different pulse frequencies, which changes
 * 			the size of the of the frame width (50Hz creates 20ms frames but at 100Hz your frames
 * 			are 10ms).
 *
 * 			It also offers a 12-bit resolution, which means that the pwm values you can send range
 * 			from 0 to 4095.
 *
 * 			That means that at 50Hz, with frames that are 20ms wide, a pulse value of 1 represents
 * 			a 1/4096 of 20ms, which is 4.88 micro seconds.  The value required to send a 1ms pulse
 * 			should therefore be 4096 (resolution) / 20ms = 204.8 (205).
 *
 * 			Based on that, I know that my neutral position (1.5ms) should be ~308.  If I were to change
 * 			my frequency to 100Hz, however, the neutral value would be 616!
 *
 * 			This is why the PwmServo object takes a base configuration from the instantiator, and then
 * 			adjusts its own internal configuration based on the actual frequency of the controller.
 *
 * 			This is also why it is highly recommended to send speed requests using percentages rather
 * 			than straight PWM values.
 *
 * 			ALSO IMPORTANT, because the vehicle has a certain weight and the motor driven by the Servo
 * 			can respond in different ways, there is a Min value for forward and reverse values that
 * 			really needs to be set based on your own vehicle.
 *
 * 		-	and PCA9685* pointing to the PCA9685 PWM controller in use.  I struggled a little bit with
 * 			this one.  Ideally, you should really take a generic 'pwm controller' object offering a
 * 			standard interface and then implement the PCA9685 using that interface but a few things
 * 			happened:
 * 			1)	I am still experimenting and not ready to solidify a generic interface when I am not
 * 				100% sure of how this should work as a final product
 * 			2)	I hadn't touched c++ in 20 years and I am still catching up and refreshing my memory
 * 			3)	I got lazy and just wanted to see those wheels spin on that car :)
 *
 * -----------------------------------------------------------------------------------------------------
 */

PwmServo::PwmServo (PwmServoConfig* config, PCA9685* controller)
{
	ready = 0;
	baseConfig = config;
	currentPos = -1;
	pwm = controller;
	init();
}




/**
 * -----------------------------------------------------------------------------------------------------
 * DESTRUCTOR
 * Returns servo to its initial position and terminates
 * -----------------------------------------------------------------------------------------------------
 */

PwmServo::~PwmServo(void)
{
	pwm->setPwm(cfg.channel, cfg.posInit);
}




/**
 * -----------------------------------------------------------------------------------------------------
 * int init (void)
 * Initializes the Servo by initializing its internal configuration based on the provided
 * config and setting it to its initial position.
 * -----------------------------------------------------------------------------------------------------
 */

int PwmServo::init(void)
{
	int success = 0;		// will be used to verify the initialization's final success
	ready = 0;				// Cancels the servo's readiness until this is finished

	// if the PWM controller is not ready and cannot be reset, then return 0 (failed)
	if (! pwm->isReady()) pwm->reset();
	if (! pwm->isReady()) return 0;


	// Sets internal configuration, taking actual PWM frequency and resolution into consideration
	// to calculate the ratios for the values to send the PWM controller.
	cfg.frequency = pwm->getFrequency();
	cfg.resolution = pwm->getResolution();
	cfg.channel = baseConfig->channel;
	cfg.posInit = (int) baseConfig->posInit;
	cfg.posStraight = (int) baseConfig->posStraight;
	cfg.posMinLeft = (int) baseConfig->posMinLeft;
	cfg.posMaxLeft = (int) baseConfig->posMaxLeft;
	cfg.posMinRight = (int) baseConfig->posMinRight;
	cfg.posMaxRight = (int) baseConfig->posMaxRight;
	
	/*
	double resolutionFactor = cfg.resolution / baseConfig->resolution;
	double frameSizeFactor = (1000 / cfg.frequency) / (1000 / baseConfig->frequency);
	cfg.posInit = (int) baseConfig->posInit * resolutionFactor * frameSizeFactor;
	cfg.posStraight = (int) baseConfig->posStraight * resolutionFactor * frameSizeFactor;
	cfg.posMinLeft = (int) baseConfig->posMinLeft * resolutionFactor * frameSizeFactor;
	cfg.posMaxLeft = (int) baseConfig->posMaxLeft * resolutionFactor * frameSizeFactor;
	cfg.posMinRight = (int) baseConfig->posMinRight * resolutionFactor * frameSizeFactor;
	cfg.posMaxRight = (int) baseConfig->posMaxRight * resolutionFactor * frameSizeFactor;
	*/


	// Use this to quickly check if the configuration was set properly
	// cout << "Sending " << cfg.posInit << " to the car's servo for initialization." << endl;


	// Place the servo in its initial position
	success = pwm->setPwm(cfg.channel, cfg.posInit);
	if (success) {
		currentPos = cfg.posInit;
		ready = 1;
	}
	return ready;
}




/**
 * -----------------------------------------------------------------------------------------------------
 * int isReady (void)
 * Returns the value of 'ready', which indicates whether it is safe to operate the servo or not
 * -----------------------------------------------------------------------------------------------------
 */

int PwmServo::isReady(void) { return ready; }




/**
 * -----------------------------------------------------------------------------------------------------
 * void printStatus (void)
 * Prints a detailed status of the servo to the standard output
 * -----------------------------------------------------------------------------------------------------
 */

void PwmServo::printStatus (void)
{
	cout << "SERVO STATUS" << endl;
	cout << "------------" << endl << endl;
	cout << "Is Ready      :  " << (isReady() ? "Yes" : "No") << endl;
	cout << "Frequency     :  " << cfg.frequency << endl;
	cout << "Resolution    :  " << cfg.resolution << endl;
	cout << "Initial Pos   :  " << cfg.posInit << endl;
	cout << "Straight      :  " << cfg.posStraight << endl;
	cout << "Min Left      :  " << cfg.posMinLeft << endl;
	cout << "Max Left      :  " << cfg.posMaxLeft << endl;
	cout << "Min Right     :  " << cfg.posMinRight << endl;
	cout << "Max Right     :  " << cfg.posMaxRight << endl;
	cout << endl;
}




/**
 * -----------------------------------------------------------------------------------------------------
 * int setPwm (int value)
 * Can be used to set the PWM to an arbitrary value.  Avoid using unless you absolutely know what
 * you are doing. The risk is that the pwm controller might be working at a different frequency than
 * you think and using absolute values bypasses the software's ability to scale values up/down to
 * take that into consideration.
 * -----------------------------------------------------------------------------------------------------
 */

int PwmServo::setPwm (int value)
{
	int success = 0;


	// If the servo isn't ready or of the PWM controller isn't ready, return 0 (fail)
	if ((! ready) || (! pwm->isReady())) 
		return 0;


	// Send command to PWM
	success = pwm->setPwm(cfg.channel, value);


	// On success, assign the last known good value to currentPos
	if (success)
	{
		currentPos = value;
	}
	else
	{
		printf("Failed to set servo.\n");
	}


	return success;
}

//-1.0f full left, 1.0f full right. 0.0f center
void PwmServo::set(float a)
{
	if( a >= -1.0f && a <= 1.0f)
	{
		int low = cfg.posMaxLeft;
		int hi = cfg.posMaxRight;
		int mid = cfg.posStraight;

		if( a == 0.0f)
		{
			setPwm(mid);
		}
		else if( a > 0.0f)
		{
			float delta = (float)(hi - mid);
			float dval = delta * a;
			int val = (int)(mid + dval);
			setPwm(val);
		}
		else if( a < 0.0f )
		{
			float delta = (float)(mid - low);
			float dval = delta * -a;
			int val = (int)(mid - dval);
			setPwm(val);
		}
	}
}



/**
 * -----------------------------------------------------------------------------------------------------
 * int straight (void)
 * Sets the servo in 'straight' position, as defined in the configuration.  It is the '0%' position.
 * -----------------------------------------------------------------------------------------------------
 */

int PwmServo::straight (void)
{
	int success = 0;


	// If the servo is not ready or if the PWM controller is not ready, return 0 (failed)
	if ((! ready) || (! pwm->isReady())) return 0;

	success = pwm->setPwm(cfg.channel, cfg.posStraight);
	if (success) currentPos = cfg.posStraight;
	return success;
}




/**
 * -----------------------------------------------------------------------------------------------------
 * int straight (void)
 * Sets the servo in 'straight' position, as defined in the configuration.  It is the '0%' position.
 * -----------------------------------------------------------------------------------------------------
 */

int PwmServo::leftPct (int percent)
{
	int value;
	double pctValue = (abs((double) cfg.posMaxLeft - (double) cfg.posMinLeft) / 100) * (double) percent;
	if (percent == 0) value = cfg.posStraight;
	else value = (fmin(cfg.posMinLeft, cfg.posMaxLeft) == cfg.posMinLeft ? cfg.posMinLeft + pctValue : cfg.posMinLeft - pctValue);
	cout << "Moving to position left " << pctValue << "  " << percent << "% with value " << value << " based on min " << cfg.posMinLeft << " and max " << cfg.posMaxLeft << endl;
	if (ready && pwm->isReady()) {
		pwm->setPwm(cfg.channel, value);
		currentPos = value;
	}
	return 1;
}

/****************************************************************/
int PwmServo::rightPct (int percent)
{
	int value;
	double pctValue = (abs((double) cfg.posMaxRight - (double) cfg.posMinRight) / 100) * (double) percent;
	if (percent == 0) value = cfg.posStraight;
	else value = (fmin(cfg.posMinRight, cfg.posMaxRight) == cfg.posMinRight ? cfg.posMinRight + pctValue : cfg.posMinRight - pctValue);
	cout << "Moving to position right " << percent << "% with value " << value << " based on min " << cfg.posMinRight << " and max " << cfg.posMaxRight << endl;
	if (ready && pwm->isReady()) {
		pwm->setPwm(cfg.channel, value);
		currentPos = value;
	}
	return 1;
}


/****************************************************************/
int PwmServo::turnPct (int percent)
{
	cout << "Turning to " << percent << endl;
	if (! percent) return straight();
	else if (percent < 0) return leftPct(abs(percent));
	else return rightPct (percent);
}

/****************************************************************/
int PwmServo::getPwm (void)
{
	int result = -1;
	if (ready && pwm->isReady())
		result = pwm->getPwm(cfg.channel);
	return result;
}

