//wiringPiLite.h

#ifndef __WIRING_PI_LITE__
#define __WIRING_PI_LITE__

void delay(unsigned int duration);

int wiringPiI2CReadReg8 (int fd, int reg);

int wiringPiI2CReadReg16 (int fd, int reg);

int wiringPiI2CWriteReg8 (int fd, int reg, int value);

int wiringPiI2CWriteReg16 (int fd, int reg, int value);


#endif //
