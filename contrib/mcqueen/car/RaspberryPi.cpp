/*
 * RaspberryPi.cpp
 *
 *  Created on: Mar 26, 2014
 *      Author: fabien papleux
 */

#include <iostream>
#include <string.h>
#include <fstream>
#include <sstream>
#include <wiringPi.h>
#include "RaspberryPi.h"
#include "Gpio.h"
#include "I2cBus.h"

using namespace std;


RaspberryPi::RaspberryPi (void)
{
	ready = 0;
	cpuModel = "";
	cpuRevision = 0;
	cpuSerial = "";
	version = 0;
	i2c = NULL;
	gpio = NULL;
	init();
}




RaspberryPi::~RaspberryPi (void)
{
	if (i2c) delete i2c;
	if (gpio) delete gpio;
}




int RaspberryPi::isReady (void)
{
	return ready;
}




int RaspberryPi::init (void)
{
	int success = 0;
	ready = 0;
	getCpuInfo();
	if (wiringPiSetup() != -1) success = 1;
	if (success) success = initGpio();
	if (success) success = initI2cBus();
	if (success) ready = 1;
	return ready;
}




int RaspberryPi::initI2cBus (void)
{
	if (! i2c) i2c = new I2cBus();
	if (! i2c->isReady()) i2c->init();
	return i2c->isReady();
}




int RaspberryPi::initGpio (void)
{
	if (version == 0) getCpuInfo();
	if ((version != 0) && (! gpio)) gpio = new Gpio(version);
	if (! gpio->isReady()) gpio->init();
	return gpio->isReady();
}




I2cBus *RaspberryPi::getI2cBus (void)
{
	return i2c;
}




Gpio *RaspberryPi::getGpio (void)
{
	return gpio;
}

Pin	*RaspberryPi::getPin (int pinNumber)
{
	return gpio->getPin(pinNumber);
}



void RaspberryPi::getCpuInfo (void)
{
	cpuModel = "";
	cpuRevision = 0;
	cpuSerial = "";
	version = 0;

	string line, l;
	stringstream ss;
	string cpuInfoPath = CPUINFO;
	ifstream f(cpuInfoPath.c_str());
	if (! f.is_open()) return;
	while (getline(f, line))
	{
		if (line.find("Revision") != string::npos)
		{
			ss << line;
			ss >> l >> l >> hex >> cpuRevision >> dec;
		}
		if (line.find("model name") != string::npos)
			cpuModel = line.substr(line.find(":") + 2, line.length() - line.find(":") + 2);
		if (line.find("Serial") != string::npos)
			cpuSerial = line.substr(line.find(":") + 2, line.length() - line.find(":") + 2);
	}
	f.close();
	if (cpuRevision) {
		/**
		 * Setting Raspberry Pi board revision based on Gordon Henderson's table used in wiringPi
		 * 	0000 - Error
		 *	0001 - Not used
		 *	0002 - Rev 1
		 *	0003 - Rev 1
		 *	0004 - Rev 2 (Early reports?
		 *	0005 - Rev 2 (but error?)
		 *	0006 - Rev 2
		 *	0008 - Rev 2 - Model A
		 *	000e - Rev 2 + 512MB
		 *	000f - Rev 2 + 512MB
		 *	Basically, version 1 if Revision is 1,2 or 3
		 */
		if (cpuRevision >= 1 && cpuRevision <= 3) version = 1;
		else version = 2;
	}
}


int RaspberryPi::getRevision (void)
{
	if (cpuRevision) return cpuRevision;
	getCpuInfo();
	return cpuRevision;
}

int RaspberryPi::getVersion (void)
{
	if (version) return version;
	getCpuInfo();
	return version;
}

const char *RaspberryPi::getModel (void)
{
	if (cpuModel != "") return cpuModel.c_str();
	getCpuInfo();
	return cpuModel.c_str();
}

const char *RaspberryPi::getSerial (void)
{
	if (cpuSerial != "") return cpuSerial.c_str();
	getCpuInfo();
	return cpuSerial.c_str();
}


void RaspberryPi::printStatus (void)
{
	cout << "Raspberry Pi Status" << endl;
	cout << "-------------------" << endl;
	cout << endl;
	cout << "Is Ready       : " << (ready ? "Yes" : "No") << endl;
	cout << "Model Name     : " << cpuModel << endl;
	cout << "Serial Number  : " << cpuSerial << endl;
	cout << "Board revision : " << cpuRevision << endl;
	cout << "Board version  : " << version << endl;
	cout << "Gpio           : " << (gpio != NULL ? string("Present ").append((gpio->isReady() ? "and Ready" : "but Not Ready"))  : "Absent") << endl;
	cout << "I2C Bus        : " << (i2c != NULL ? string("Present ").append((i2c->isReady() ? "and Ready" : "but Not Ready"))  : "Absent") << endl;
	cout << endl;
	if (gpio) gpio->printStatus();
	if (i2c) i2c->printStatus();
}
