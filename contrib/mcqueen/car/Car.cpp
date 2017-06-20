/*
 * Car.cpp
 *
 *  Created on: Mar 23, 2014
 *      Author: fabien papleux
 */

#include "SharkConfig.h"
#include <iostream>
#include <string>

#if ENABLE_WIRING_PI 
	#include <wiringPi.h>
#else
	#include "wiringPiLite.h"
#endif

#include "PwmEsc.h"
#include "PwmServo.h"
#include "PCA9685.h"
#include "I2cBus.h"

#if ENABLE_WIRING_PI
	#include "RaspberryPi.h"
#endif

#include "Car.h"

using namespace std;




/**
 * ------------------------------------------------------------------------------------------------------------
 * CONSTRUCTOR
 * Creates the car object and starts its initialization
 * ------------------------------------------------------------------------------------------------------------
 */

Car::Car(void)
{
	#if ENABLE_WIRING_PI
	pi = NULL;
	#endif

	i2c = NULL;
	pwm = NULL;
	servo = NULL;
	esc = NULL;
	ready = 0;
}




/**
 * ------------------------------------------------------------------------------------------------------------
 * DESTRUCTOR
 * Closes down the car's electronics and frees memory
 * ------------------------------------------------------------------------------------------------------------
 */

Car::~Car(void)
{
	if (servo) delete servo;
	if (esc) delete esc;
	if (pwm) delete pwm;

	#if ENABLE_WIRING_PI
	if (pi) delete pi;
	#endif
}




/**
 * ------------------------------------------------------------------------------------------------------------
 * int init (void)
 * Initializes the car.
 * You should be able to call this again if something fails so it returns an int to determine success and
 * it checks whether its components are existing objects before (re-)creating them.
 * ------------------------------------------------------------------------------------------------------------
 */

int Car::init(PwmServoConfig* pSteeringConfig, 
		PwmEscConfig* pEscConfig,
		PWMBoardConfig* pBoardConfig)
{
	ready = 0;		// Car is not ready yet

//manual config if you like it.
/*
	// Configuration of the car's servo
	servoConfig.channel 	= 2;
	servoConfig.frequency 	= 60;
	servoConfig.resolution 	= 4096;
	servoConfig.posInit 	= 400;
	servoConfig.posStraight = 400;
	servoConfig.posMinLeft 	= 310;
	servoConfig.posMaxLeft 	= 310;
	servoConfig.posMinRight = 530;
	servoConfig.posMaxRight = 530;


	// Configuration of the car's ESC
	escConfig.channel 		= 4;
	escConfig.frequency 	= 60;
	escConfig.resolution 	= 4096;
	escConfig.posInit 		= 400;
	escConfig.posIdle 		= 400;
	escConfig.posMinForward	= 450;
	escConfig.posMaxForward	= 600;
	escConfig.posMinReverse	= 350;
	escConfig.posMaxReverse	= 200;
*/

	//otherwize deep copying in our config
	servoConfig = *pSteeringConfig;
	escConfig = *pEscConfig;

	// Initialize the Raspberry Pi (generically, this is the car's main microcontroller.
	// It could have been an Arduino or something else.
#if ENABLE_WIRING_PI
	if (pi) delete pi;
	pi = new RaspberryPi();

	// if the microController did not initialized properly, exit with failure (0)
	if ((! pi) || (! pi->isReady())) return 0;


	// the I2cBus is automatically initalized by the Raspberry Pi.  We are just reassigning the pointer
	i2c = 	pi->getI2cBus();
#else

	//we are not on the Pi, so make our own I2C bus object
	if(i2c == NULL)
	 	i2c = new I2cBus(pBoardConfig->device_file.c_str());
	if (! i2c->isReady()) 
		i2c->init(pBoardConfig->device_file.c_str());

	cout << "I2C: " << i2c << endl;
#endif

	// (Re-)creating and (Re-)initializing the PWM controller (PCA9685)
	// using standard PCA9685 configuration: I2C address 0x40, and using 50Hz as the PWM pulse frequency
	// Also inserting a small delay to let additional components get in place
	// Also, the PCA9685 board is powered through the pi's pin 7 so we turn it on here.
#if ENABLE_WIRING_PI
	pi->getPin(7)->setValue(HIGH);
#endif	

	if (pwm) delete pwm;
	pwm = 	new PCA9685 	(i2c, pBoardConfig->i2c_address, servoConfig.frequency);
	delay(25);


	// (re-)Initializing the car's servo
	if (servo) delete servo;
	servo = new PwmServo 	(&servoConfig, pwm);
	delay(25);


	// (re-)Initializing the car's Electronic Speed Controller (ESC)
	if (esc) delete esc;
	esc = 	new PwmEsc		(&escConfig, pwm);
	delay(25);

	// Determining if all systems are a go before declaring the car 'ready'
	if (i2c->isReady() && pwm->isReady() && servo->isReady() && esc->isReady()) ready = 1;
	return ready;
}




/**
 * ------------------------------------------------------------------------------------------------------------
 * int isReady (void)
 * Returns the value of 'ready'.  Ready = 1 (True) if it is ready, 0 otherwise.
 * ------------------------------------------------------------------------------------------------------------
 */

int Car::isReady (void) { return ready; }





/**
 * ------------------------------------------------------------------------------------------------------------
 * PwmServo *getServo (void)
 * Returns a pointer to the Servo object
 * ------------------------------------------------------------------------------------------------------------
 */

PwmServo *Car::getServo (void) { return servo; }





/**
 * ------------------------------------------------------------------------------------------------------------
 * PwmEsc *getEsc (void)
 * Returns a pointer to the ESC object
 * ------------------------------------------------------------------------------------------------------------
 */

PwmEsc *Car::getEsc (void) { return esc; }





/**
 * ------------------------------------------------------------------------------------------------------------
 * PCA9685 *getPCA9685 (void)
 * Returns a pointer to the PCA9685 object (PWM controller)
 * ------------------------------------------------------------------------------------------------------------
 */

PCA9685 *Car::getPCA9685 (void) { return pwm; }





/**
 * ------------------------------------------------------------------------------------------------------------
 * I2cBus *getI2cBus (void)
 * Returns a pointer to the I2C bus object
 * ------------------------------------------------------------------------------------------------------------
 */

I2cBus *Car::getI2cBus (void) { return i2c; }





/**
 * ------------------------------------------------------------------------------------------------------------
 * RaspberryPi *getRaspberryPi (void)
 * Returns a pointer to the RaspberryPi object (the microcontroller used on the car)
 * ------------------------------------------------------------------------------------------------------------
 */

#if ENABLE_WIRING_PI
RaspberryPi *Car::getRaspberryPi (void) { return pi; }
#endif




/**
 * ------------------------------------------------------------------------------------------------------------
 * void printStatus (void)
 * Prints the object's status to the standard output
 * ------------------------------------------------------------------------------------------------------------
 */

void Car::printStatus (void)
{
	string out;
	cout << "CAR DETAILED STATUS" << endl;
	cout << "-------------------" << endl << endl;
	cout << "Is Ready              : " << (ready ? "Yes" : "No") << endl;
#if ENABLE_WIRING_PI
	cout << "Raspberry Pi          : " << (pi    ? string("Present").append((pi->isReady()    ? " and Ready" : " but not ready")) : "Absent") << endl;
#endif
	cout << "I2C Bus               : " << (i2c   ? string("Present").append((i2c->isReady()   ? " and Ready" : " but not ready")) : "Absent") << endl;
	cout << "PCA9685 (PWM)         : " << (pwm   ? string("Present").append((pwm->isReady()   ? " and Ready" : " but not ready")) : "Absent") << endl;
	cout << "Servo (direction)     : " << (servo ? string("Present").append((servo->isReady() ? " and Ready" : " but not ready")) : "Absent") << endl;
	cout << "ESC (throttle)        : " << (esc 	 ? string("Present").append((esc->isReady()   ? " and Ready" : " but not ready")) : "Absent") << endl;
	cout << endl;

#if ENABLE_WIRING_PI
	pi->printStatus();
#endif

	pwm->printStatus();
	servo->printStatus();
	esc->printStatus();
}


//-1.0f full left, 1.0f full right. 0.0f center
void 	Car::setSteering(float percent)
{
	servo->set(percent);
}

//-1.0f full reverse, 1.0f full forward. 0.0f idle
void 	Car::setThrottle(float percent)
{
	esc->set(percent);
}




/**
 * ------------------------------------------------------------------------------------------------------------
 * int turnRightPct (int percent)
 * Tells the servo object to turn the car's wheels to the right by 'percent' percent of its range
 * Returns 0 if failed, 1 otherwise
 * ------------------------------------------------------------------------------------------------------------
 */

int	Car::turnRightPct (int percent)
{
	return servo->rightPct(percent);
}






/**
 * ------------------------------------------------------------------------------------------------------------
 * int turnLeftPct (int percent)
 * Tells the servo object to turn the car's wheels to the left by 'percent' percent of its range
 * Returns 0 if failed, 1 otherwise
 * ------------------------------------------------------------------------------------------------------------
 */

int	Car::turnLeftPct (int percent)
{
	return servo->leftPct(percent);
}





/**
 * ------------------------------------------------------------------------------------------------------------
 * int straight (void)
 * Tells the servo object to turn the car's wheels to be in the straight position.
 * Returns 0 if failed, 1 otherwise
 * ------------------------------------------------------------------------------------------------------------
 */

int	Car::straight (void)
{
	return servo->straight();
}





/**
 * ------------------------------------------------------------------------------------------------------------
 * int turnPct (int percent)
 * Tells the servo object to turn the car's wheels to 'percent' percent of its range, using
 * -100% to +100% as a reference, -100% behind all the way to the left, to +100% all the way to the right
 * A value of 0 indicates 'straight'
 * Returns 0 if failed, 1 otherwise
 * ------------------------------------------------------------------------------------------------------------
 */

int	Car::turnPct (int percent)
{
	return servo->turnPct(percent);
}





/**
 * ------------------------------------------------------------------------------------------------------------
 * int turn (int pwmValue)
 * Tells the servo object to set its PWM value to pwmValue.  This is a straight passthrough routine used to
 * bypass the higher level logic in case a special need arises.  It has been mainly used for debugging and
 * its use should be avoided as other items in the car could alter the frequency or the resolution of the
 * PWM controller, and thus the appropriate values that should be sent to the servo.
 * Returns 0 if failed, 1 otherwise
 * ------------------------------------------------------------------------------------------------------------
 */

int	Car::turn (int pwmValue)
{
	return servo->setPwm(pwmValue);
}





/**
 * ------------------------------------------------------------------------------------------------------------
 * int forwardPct (int percent)
 * Tells the ESC object to set the car's thrust forward by 'percent' percent of its range
 * Returns 0 if failed, 1 otherwise
 * ------------------------------------------------------------------------------------------------------------
 */

int	Car::forwardPct (int percent)
{
	return esc->forwardPct(percent);
}





/**
 * ------------------------------------------------------------------------------------------------------------
 * int reversePct (int percent)
 * Tells the ESC object to set the car's thrust in reverse to 'percent' percent of its range
 * Returns 0 if failed, 1 otherwise
 * ------------------------------------------------------------------------------------------------------------
 */

int	Car::reversePct (int percent)
{
	return esc->reversePct(percent);
}





/**
 * ------------------------------------------------------------------------------------------------------------
 * int stop (void)
 * Tells the car to stop (move the ESC back to neutral). Note that it does not mean that any
 * braking ability is applied.  In a future version I might add a hardStop routine if necessary
 * that will spin the motor in reverse or forward to actively brake.
 * Returns 0 if failed, 1 otherwise
 * ------------------------------------------------------------------------------------------------------------
 */

int	Car::stop (void)
{
	return esc->stop();
}





/**
 * ------------------------------------------------------------------------------------------------------------
 * int speedPct (int percent)
 * Tells the ESC object to set the car's thrust to 'percent' percent of its range, from
 * -100% being full reverse and +100% being full forward.  0 is idle/stop.
 * Returns 0 if failed, 1 otherwise
 * ------------------------------------------------------------------------------------------------------------
 */

int	Car::speedPct (int percent)
{
	return esc->speedPct(percent);
}
