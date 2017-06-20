/*
 * PwmEsc.cpp
 *
 *  Created on: Mar 26, 2014
 *      Author: fabien papleux
 *
 *  I could have probably written this code to be a little more generically but I struggled
 *  with abstracting the hardware layer. Instead, I opted for layering objects so they "connect"
 *  logically in the software as they do in real life.  You command the software object the
 *  way you want to command it, and that object then in turn knows how to relay that command
 *  to its physical self through its first uplink.
 *
 *  For instance, I connect to my ESC like this:
 *
 *  Raspberry Pi >> I2C bus >> PCA9685 PWM controller >> channel 9 >> ESC
 *
 *  I guess you could plug the ESC straight into a Raspberry Pi PWM pin or you could use
 *  an SPI controller instead.  The point is that as long as you maintain a simple interface,
 *  you should be able to adjust the configuration with minimal changes.
 *
 *  I have also struggled a little bit with the 'channel' value that I have assigned to this
 *  configuration.  In principle, which channel the ESC is connected to is a function of the
 *  controller rather than the ESC itself.  However, I didn't want to implement pwm controllers
 *  for each channels right now to keep it simple so it's a quirk that will probably have to be
 *  solved later.
 *
 *  If you are editing this on a platform that does not have the wiringPi libraries loaded, you will
 *  see errors in the code.  That is normal.  I compile on the raspberry pi directly and it works.
 *
 */

#include "SharkConfig.h"
#include <iostream>
#include <cmath>
#include <stdlib.h>
#if ENABLE_WIRING_PI 
	#include <wiringPi.h>
#else
	#include "wiringPiLite.h"
#endif
#include "PCA9685.h"
#include "PwmEsc.h"

using namespace std;



/**
 * -----------------------------------------------------------------------------------------------------
 * CONSTRUCTOR
 * initializes the ESC (if the PWM controller has already been initialized)
 * Receives:
 * 		-	A PwmEscConfig* that should contain the user-set values to determine the PWM values
 * 			to send the controller to achieve various tasks.  That configuration will be saved as
 * 			the base configuration.  PwmEscConfig is a struct defined in PwmEsc.h.
 *
 *			The base configuration needs to be adjusted based on the actual PWM controller
 * 			configuration because PWM controllers can take different forms:  At the end of the day
 * 			a typical ESC will be on neutral with a 1.5ms pulse, full reverse with a 1ms pulse and
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
 * 			This is why the PwmEsc object takes a base configuration from the instantiator, and then
 * 			adjusts its own internal configuration based on the actual frequency of the controller.
 *
 * 			This is also why it is highly recommended to send speed requests using percentages rather
 * 			than straight PWM values.
 *
 * 			ALSO IMPORTANT, because the vehicle has a certain weight and the motor driven by the ESC
 * 			can respond in different ways, there is a Min value for forward and reverse values that
 * 			really needs to be set based on your own vehicle.  For instance, my neutral seems to be
 * 			at 335 (instead of theoretical 308) but then it takes until 357 for the car to even
 * 			start moving forward.  Values from 335 to 357 are lost.  So, when you calculate the %
 * 			thrust you send to the motor, the range from 0 to 100% should not be mapped to 335 (neutral)
 * 			to 450 (my max forward), but rather from 357 (min forward) to 450 (max forward).
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

PwmEsc::PwmEsc (PwmEscConfig* config, PCA9685* controller)
{
	ready = 0;
	baseConfig = config;
	lastPwm = -1;
	pwm = controller;
	init();
}




/**
 * -----------------------------------------------------------------------------------------------------
 * DESTRUCTOR
 * - Puts the ESC in initial position
 * -----------------------------------------------------------------------------------------------------
 */

PwmEsc::~PwmEsc(void)
{
	// Puts the ESC in initial position (assuming initial position is idle)
	pwm->setPwm(cfg.channel, cfg.posInit);
}




/**
 * -----------------------------------------------------------------------------------------------------
 * init (void)
 * Initializes the ESC and ensures that it can communicate with it
 * -----------------------------------------------------------------------------------------------------
 */

int PwmEsc::init(void)
{
	int success;		// used to verify command returns from the PWM controller

	// start by voiding the readiness of the object
	ready = 0;


	// check if the PWM controller is ready.  If it isn't, try to reset it once.  If it still isn't
	// ready, abort initialization and return 0 (failed)
	if (! pwm->isReady()) pwm->reset();
	if (! pwm->isReady()) return 0;


	// Assigns the controller's current frequency and resolution to the ESC's internal configuration
	// to calculate updated PWM values from the base configuration (see main comment as to why)
	cfg.frequency = pwm->getFrequency();
	cfg.resolution = pwm->getResolution();
	cfg.channel = baseConfig->channel;


	// Adjusts internal configuration based on actual PWM controller configuration
	double resolutionFactor = cfg.resolution / baseConfig->resolution;
	double frameSizeFactor = (1000 / cfg.frequency) / (1000 / baseConfig->frequency);
	cfg.posInit = (int) baseConfig->posInit * resolutionFactor * frameSizeFactor;
	cfg.posIdle = (int) baseConfig->posIdle * resolutionFactor * frameSizeFactor;
	cfg.posMinForward = (int) baseConfig->posMinForward * resolutionFactor * frameSizeFactor;
	cfg.posMaxForward = (int) baseConfig->posMaxForward * resolutionFactor * frameSizeFactor;
	cfg.posMinReverse = (int) baseConfig->posMinReverse * resolutionFactor * frameSizeFactor;
	cfg.posMaxReverse = (int) baseConfig->posMaxReverse * resolutionFactor * frameSizeFactor;


	// Use this cout to see if the initial position calculated above makes sense.
	// cout << "Sending " << cfg.posInit << " to the car's servo for initialization." << endl;


	// PWM ESC initializing sequence (arming): max forward, max reverse, then idle.
	// not checking for the success of those commands here because if they didn't work,
	// we will see that later when we set the initial position
	int delay_duration = 5;
	
	pwm->setPwm(cfg.channel, cfg.posMaxForward);
	delay(delay_duration);
	pwm->setPwm(cfg.channel, cfg.posMaxReverse);
	delay(delay_duration);
	pwm->setPwm(cfg.channel, cfg.posIdle);
	delay(delay_duration);

	printf("initing ESC on channel %d, with seq %d - %d - %d\n", 
		cfg.channel, cfg.posMaxForward, cfg.posMaxReverse, cfg.posIdle);


	// Send command to set the ESC is the initial position (as specified in the config)
	success = pwm->setPwm(cfg.channel, cfg.posInit);
	delay(delay_duration);

	// On success, save the last good PWM value and set the ready flag to 1 (success)
	if (success)
	{
		lastPwm = cfg.posInit;
		ready = 1;
	}
	return success;
}




/**
 * -----------------------------------------------------------------------------------------------------
 * isReady (void)
 * returns the value of 'ready'.  Ready is set to 1 when the ESC has been successfully initialized
 * and when no other factors are known to prevent its function.
 * -----------------------------------------------------------------------------------------------------
 */

int PwmEsc::isReady(void) { return ready; }




/**
 * -----------------------------------------------------------------------------------------------------
 * printStatus (void)
 * Sends detailed object status to the standard output.  Use for debugging when necessary.
 * -----------------------------------------------------------------------------------------------------
 */

void PwmEsc::printStatus (void)
{
	cout << "PWM ESC STATUS" << endl;
	cout << "--------------" << endl << endl;
	cout << "Is Ready      : " << (isReady() ? "Yes" : "No") << endl;
	cout << "Frequency     : " << cfg.frequency << endl;
	cout << "Resolution    : " << cfg.resolution << endl;
	cout << "Last good PWM : " << lastPwm << endl;
	cout << "Initial Pos   : " << cfg.posInit << endl;
	cout << "Idle          : " << cfg.posIdle << endl;
	cout << "Min Forward   : " << cfg.posMinForward << endl;
	cout << "Max Forward   : " << cfg.posMaxForward << endl;
	cout << "Min Backward  : " << cfg.posMinReverse << endl;
	cout << "Max Backward  : " << cfg.posMaxReverse << endl;
	cout << endl;
}




/**
 * -----------------------------------------------------------------------------------------------------
 * setPwm (int value)
 * Sends a specific pulse value to the PWM controller.  Be careful when you use this method as it
 * does not check whether the value has been adjusted for the current PWM controller configuration.
 * -----------------------------------------------------------------------------------------------------
 */

int PwmEsc::setPwm (int value)
{
	int success;		// captures the response of the pwm controller


	// If the Esc is not ready, or if the PWM controller is not ready, return 0 (failed)
	if ((! ready) && (! pwm->isReady())) return 0;


	// Sends the command to the controller and returns the controller's return value
	success = pwm->setPwm(cfg.channel, value);
	if (success) lastPwm = value;
	return success;
}


//-1.0f full reverse, 1.0f full forward. 0.0f idle
void PwmEsc::set(float a)
{
	if( a >= -1.0f && a < 1.0f)
	{
		int low = cfg.posMinReverse;
		int hi = cfg.posMinForward;
		int mid = cfg.posIdle;

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
 * stop ()
 * Sends the stop pwm value to the controller, telling the ESC to set to neutral.
 * Note that this stops the vehicle but it does not break.  To actively break, you would need to
 * spin the motor in reverse until it reaches a stopping position.
 *
 * I will later implement a hardStop and/or a brake method to accomplish this but it is not
 * necessary for now.
 * -----------------------------------------------------------------------------------------------------
 */

int PwmEsc::stop (void)
{
	int success;		// captures the response of the pwm controller


	// If the Esc is not ready, or if the PWM controller is not ready, return 0 (failed)
	if ((! ready) && (! pwm->isReady())) return 0;


	success = pwm->setPwm(cfg.channel, cfg.posIdle);
	if (success) lastPwm = cfg.posIdle;
	return success;
}




/**
 * -----------------------------------------------------------------------------------------------------
 * forwardPct (int percent)
 * Moves the car forward.  Unlike the reverse method which might need to send stop commands before
 * going in reverse, forward can be applied any time.
 * -----------------------------------------------------------------------------------------------------
 */

int PwmEsc::forwardPct (int percent)
{
	int success;		// captures the response of the pwm controller
	int value;

	// If the ESC is not ready, or if the PWM controller is not ready, return 0 (failed)
	if ((! ready) || (! pwm->isReady())) return 0;


	// If the percentage is zero, then stop the car instead
	if (percent == 0) return stop();


	// Adjust the target PWM value to send to the controller, taking into account that it could be that some ESC's could
	// have reversed values (MIN > MAX or MAX > MIN)
	double pctValue = (abs((double) cfg.posMaxForward - (double) cfg.posMinForward) / 100) * (double) percent;
	value = (fmin(cfg.posMinForward, cfg.posMaxForward) == cfg.posMinForward ? cfg.posMinForward + pctValue : cfg.posMinForward - pctValue);


	// in case you want to read what the command translates into, uncomment the line below
	// cout << "Moving to position forward " << percent << "% with value " << value << " based on min " << cfg.posMinForward << " and max " << cfg.posMaxForward << endl;


	// Send target PWM value command to the unit, record that value as the last PWM value sent and return that value to the caller
	success = pwm->setPwm(cfg.channel, value);
	if (success) lastPwm = value;
	return success;
}




/**
 * -----------------------------------------------------------------------------------------------------
 * reversePct (int percent)
 * Send the ESC the command to go in reverse at "percent"% of
 * the capacity of the engine.  Uses the min & max backward settings
 * of the configuration to determine the target PWM value
 * -----------------------------------------------------------------------------------------------------
 */

int PwmEsc::reversePct (int percent)
{
	int current;	// to hold the current PWM value
	int value;		// to hold the target PWM value
	int success;	// to hold the return value of the underlying PWM controller command


	// If the ESC is not ready, or if the PWM controller is not ready, return 0 (failed)
	if ((! ready) || (! pwm->isReady())) return 0;


	// If the percentage is zero, then stop the car instead
	if (percent == 0) return stop();


	// Translate the requested percentage into the adjusted PWM value relative to the MIN and MAX pwm values
	double pctValue = (abs((double) cfg.posMaxReverse - (double) cfg.posMinReverse) / 100) * (double) percent;


	// Adjust the target PWM value to send to the controller, taking into account that it could be that some ESC's could
	// have reversed values (MIN > MAX or MAX > MIN)
	value = (fmin(cfg.posMinReverse, cfg.posMaxReverse) == cfg.posMinReverse ? cfg.posMinReverse + pctValue : cfg.posMinReverse - pctValue);


	// in case you want to read what the command translates into, uncomment the line below
	// cout << "Moving to position reverse " << percent << "% with value " << value << " based on min " << cfg.posMinReverse << " and max " << cfg.posMaxReverse << endl;


	// Read the current PWM value to determine at what speed we are currently running.
	// If we are moving forward, we will first have to stop.  ESC's have a safety system that requires
	// sending a stop command before being able to go in reverse.
	current = getPwm();
	cout << "Sending reverse command " << value << " -> Current PWM value is " << current << endl;

	// If we are moving forward, send 2 stop commands. Also send 2 small delays in-between to ensure the system
	// had time to respond/acknowledge
	if (current >= cfg.posMinForward && current <= cfg.posMaxForward)
	{
		stop();
		delay(100);
		pwm->setPwm(cfg.channel, value);
		delay(100);
		stop();
		delay(100);
	}


	// Send target PWM value command to the unit, record that value as the last PWM value sent and return that value to the caller
	success = pwm->setPwm(cfg.channel, value);
	if (success) lastPwm = value;
	return success;
}




/**
 * -----------------------------------------------------------------------------------------------------
 * speedPct (int percent)
 * Send the ESC the command to go in a direction at 'percent' % of its speed capacity
 * Expected range is between -100 to 100
 * Negative percentages represent reverse speeds
 * 0 is neutral
 * Positive percentages represent forward speeds
 * -----------------------------------------------------------------------------------------------------
 */

int PwmEsc::speedPct (int percent)
{
	// Since this method will leverage other internal method to adjust speed, I am not testing
	// devices readiness here.

	// if percentage is 0, the stop the car.
	if (! percent) return stop();

	// all negative percentages are reverse speeds
	else if (percent < 0) return reversePct(abs(percent));

	// all positive percentages are forward speeds
	else return forwardPct (percent);
}




/**
 * -----------------------------------------------------------------------------------------------------
 * getPwm (void)
 * returns the current PWM stop value.
 *
 * Note: depending on controllers, it seems that start values are either always 0 or they can be set
 * by the user.  If I remember correctly, an Arduino only takes 1 value while the PCA9685 16-channel
 * PWM controller can receive both a start and a stop value.  To simplify things, I only use one and
 * always use 0 as a start value
 * -----------------------------------------------------------------------------------------------------
 */

int PwmEsc::getPwm (void)
{
	// if the ESC is not ready or if the PWM controller is not ready, return 0 (failed)
	if ((! ready) || (! pwm->isReady())) return 0;

	// return the PWM value collected from the PWM controller
	return pwm->getPwm(cfg.channel);
}

