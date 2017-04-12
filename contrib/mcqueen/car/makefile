CC=g++-4.7
CFLAGS=-c -Wall -std=c++11
CTRAIL=-lwiringPi
LDFLAGS=
LDTRAIL=-lwiringPi

all:			car

car:				RaspberryPi.o I2cBus.o Gpio.o PCA9685.o PwmEsc.o PwmServo.o Car.o app.o
	$(CC) $(LDFLAGS) Pin.o Gpio.o PwmEsc.o PwmServo.o PCA9685.o I2cBus.o RaspberryPi.o Car.o app.o -o car $(LDTRAIL)

app.o: 				Car.o app.cpp
	$(CC) $(CFLAGS) app.cpp $(CTRAIL)

Car.o:				RaspberryPi.o I2cBus.o PCA9685.o PwmServo.o PwmEsc.o Car.cpp
	$(CC) $(CFLAGS) Car.cpp $(CTRAIL)

PwmServo.o:			PCA9685.o PwmServo.cpp
	$(CC) $(CFLAGS) PwmServo.cpp $(CTRAIL)

PwmEsc.o:			PCA9685.o PwmEsc.cpp
	$(CC) $(CFLAGS) PwmEsc.cpp $(CTRAIL)

PCA9685.o:			I2cBus.o PCA9685.cpp
	$(CC) $(CFLAGS) PCA9685.cpp $(CTRAIL)

I2cBus.o:			I2cBus.cpp
	$(CC) $(CFLAGS) I2cBus.cpp $(CTRAIL)

Pin.o:				Pin.cpp
	$(CC) $(CFLAGS) Pin.cpp $(CTRAIL)

Gpio.o:				Pin.o Gpio.cpp
	$(CC) $(CFLAGS) Gpio.cpp $(CTRAIL)

RaspberryPi.o:		Gpio.o I2cBus.o RaspberryPi.cpp
	$(CC) $(CFLAGS) RaspberryPi.cpp $(CTRAIL)
	
