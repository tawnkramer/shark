/*
 * PwmLvSonarEz2.h
 *
 *  Created on: Jul 2, 2014
 *      Author: fabien papleux
 */

#ifndef PWMLVSONAREZ2_H_
#define PWMLVSONAREZ2_H_

#include "PCA9685.h"
#include "Pin.h"

/*** 
Looks like a work in progress...
 */


class PwmLvSonarEz2
{
public:
	PwmLvSonarEz2 (PCA9685 *controller, int myChannel, Pin *onPin);
	~PwmLvSonarEz2 (void);

	int		init (void);
	int		isReady (void);
	void	printStatus(void);

	int on (void);
	int off (void);
	int getPwm (void);

private:
	int ready;
	int lastPwm;					// Holds the current PWM value (position)
	int channel;
	Pin *pin;
	PCA9685 *pwm;					// pointer to a PWM controller, the address of which should be provided at creation time
};




#endif /* PWMLVSONAREZ2_H_ */
