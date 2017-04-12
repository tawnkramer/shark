/*
 * PwmEsc.h
 *
 *  Created on: Mar 26, 2014
 *      Author: fabien papleux
 */

#ifndef PWMESC_H_
#define PWMESC_H_

#include "PCA9685.h"

/*** This class requires a PWM controller class (implemented here as PCA9685) where to send the following commands:
 *
 * int reset (void)							// resets controller
 * int isReady (void)						// returns 1 if controller is ready
 * int getFrequency ()						// Returns the current frequency used by the controller to determine pulse width
 * int getResolution (void)					// Returns the current resolution of the PWM controller (PCA9685 is standard at 12)
 * int setPwm (int channel, int data)		// Sets the stop PWM value for a channel.  (the start value is to be assumed as 0)
 * int getPwm (int channel)					// Gets the current stop value of the PWM
 *
 */

/* 50Hz is the typical clock for servos. it means the controller runs 50 cycles in 1 second,
 * which means that each cycle is 20ms. Servos typically require to be high for 1ms per frame
 * to be full on one side, 1.5ms to be in the middle and 2ms to be full on the other side.
 * If your controller is a PCA9685 (like the Adafruit 16-channel 12-bit controller), the 12-bit
 * definition means that your frame's scale goes from 0 to 4095.
 *
 * If your frame is 20ms and you want to be up for 1ms, you need to be up for 1/20th of the
 * size of your frame, which would be 4096 / 20 = 205.
 */

struct PwmEscConfig {
	int channel;		// indicate what is the address/channel this Servo should provide the PWM controller to send information
	int frequency;		// PWM frequency (determines frame size) in Hz ... this is just to provide referential sizing for the data, not to set the PWM
	int resolution;		// resolution of the PWM value (PCA9685 is a 12-bit resolution so values should range from 0 to 4095, which means the value here should be 4096)
	int posInit;		// position to initialize the servo at.
	int posIdle;	// value for straight position
	int posMinForward;		// in principle, this will be idle+-1.  For ESC's, which use the same technology, there could be a difference between purely idle and the beginning of movement
	int posMaxForward;
	int posMinReverse;
	int posMaxReverse;
};

class PwmEsc
{
public:
	PwmEsc (PwmEscConfig *config, PCA9685 *controller);
	~PwmEsc (void);

	int		init (void);
	int		isReady (void);
	void	printStatus(void);

	void set(float percent);
	int forwardPct (int percent);
	int reversePct (int percent);
	int stop (void);
	int speedPct (int percent);	// going from -100% (full backward) to +100% (full forward) -- 0 being the straight/middle point

	int setPwm (int value);
	int getPwm (void);

private:
	int ready;
	int lastPwm;					// Holds the current PWM value (position)
	PwmEscConfig cfg;				// actual configuration used to manipulate the esc
	PwmEscConfig* baseConfig;		// config provided by user at initialization to be used as base during operation
	PCA9685 *pwm;					// pointer to a PWM controller, the address of which should be provided at creation time
};




#endif /* PWMESC_H_ */
