/*
 * I2cBus.h
 *
 *  Created on: Mar 25, 2014
 *      Author: fabien papleux
 */

#ifndef I2CBUS_H_
#define I2CBUS_H_

#define I2C_SLAVE	0x0703

class I2cBus
{
public:
	I2cBus (void);
	~I2cBus (void);

	int init (void);
	int isReady (void);
	void printStatus (void);
	void setSlave (int address);
	int read8 (int address, int reg);
	int read16 (int address, int reg);
	int write8 (int address, int reg, int data);
	int write16 (int address, int reg, int data);

private:
	int ready;
	int fd;
	const char *i2cPath;
	int currentSlave;
};




#endif /* I2CBUS_H_ */
