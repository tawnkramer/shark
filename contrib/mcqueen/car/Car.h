/*
 * Car.h
 *
 *  Created on: Mar 23, 2014
 *      Author: fabien papleux
 */

#ifndef CAR_H_
#define CAR_H_

#include "SharkConfig.h"
#include "PwmServo.h"
#include "PwmEsc.h"
#include "PCA9685.h"
#include "I2cBus.h"
#include <string.h>

#if ENABLE_WIRING_PI
	#include "RaspberryPi.h"
#endif

struct PWMBoardConfig
{
	PWMBoardConfig() {
		i2c_address = 0x40;
		device_file = "/dev/i2c-0";
	}

	int i2c_address;
	std::string device_file;
};

class Car
{

public:
	Car (void);
	~Car (void);
	int init (
		PwmServoConfig* pSteeringConfig, 
		PwmEscConfig* pEscConfig,
		PWMBoardConfig* pBoardConfig);			// initializes the car system, puts all systems in neutral and ready to begin operating (isReady)
		
	void printStatus (void);	// prints full status of the car

	//-1.0f full left, 1.0f full right. 0.0f center
	void 	setSteering(float percent);

	//-1.0f full reverse, 1.0f full forward. 0.0f idle
	void 	setThrottle(float percent);

	// managing direction
	int 	turnRightPct (int percent);
	int 	turnLeftPct (int percent);
	int		turnPct (int percent);
	int		turn (int pwmValue);
	int 	straight (void);

	// managing throttle
	int 	forwardPct (int percent);
	int 	reversePct (int percent);
	int		speedPct (int percent);			// ranges from -100% for full backward to +100% for full forward.
	int 	stop (void);

	// car state information
	int 	isReady (void);					// Car initialization was successful
	//int 	isMoving (void);				// throttle is non-zero
	//int 	isIdle (void);					// throttle is zero and wheels are not moving

	PwmServo	*getServo (void);			// returns a pointer to the servo for querying
	PwmEsc		*getEsc (void);				// returns a pointer to the esc
	PCA9685		*getPCA9685 (void);		 	// returns a pointer to the PwmController
	I2cBus		*getI2cBus (void);			// returns a pointer to the I2cBus object

	#if ENABLE_WIRING_PI
	RaspberryPi	*getRaspberryPi (void);		// returns a pointer to the Raspberry Pi
	#endif
	
	//int 	getCurrentDirectionPct (void);	// returns current % turn (left is negative. idle is 0. right is positive)
	//int 	getCurrentSpeedPct (void);		// returns current speed in % (backward is negative. idle is 0. forward is positive)

private:
	int 			ready;
	PwmServoConfig	servoConfig;
	PwmServo		*servo;						// points to the servo motor of the car
	PwmEscConfig	escConfig;
	PwmEsc			*esc;
	PCA9685			*pwm;
	I2cBus			*i2c;

	#if ENABLE_WIRING_PI
	RaspberryPi		*pi;
	#endif
};

#endif /* CAR_H_ */
