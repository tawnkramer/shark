/*
 * app.cpp
 *
 *  Created on: Mar 23, 2014
 *      Author: fabien papleux
 */

#include <iostream>
#include <string>
#include <stdexcept>
#include <wiringPi.h>
#include "Car.h"

using namespace std;

int main (int argv, char** args) {

	Car car;
	cout << "Car is ready ..." << endl;
	int t = 150;
	int c;

	car.printStatus();

	if (car.isReady()) {

		/*
		car.stop();
		Pin *p1 = car.getRaspberryPi()->getPin(13);
		Pin *p2 = car.getRaspberryPi()->getPin(15);
		Pin *p3 = car.getRaspberryPi()->getPin(16);
		Pin *p4 = car.getRaspberryPi()->getPin(18);

		p1->setMode(OUTPUT);
		p2->setMode(OUTPUT);
		p3->setMode(OUTPUT);
		p4->setMode(OUTPUT);

		p2->setValue(LOW);
		p3->setValue(LOW);
		p4->setValue(LOW);
		p1->setValue(HIGH);
		delay(t);

		for (c = 0; c < 10; c++) {
			p2->setValue(HIGH);
			p1->setValue(LOW);
			delay(t);
			p3->setValue(HIGH);
			p2->setValue(LOW);
			delay(t);
			p4->setValue(HIGH);
			p3->setValue(LOW);
			delay(t);
			p3->setValue(HIGH);
			p4->setValue(LOW);
			delay(t);
			p2->setValue(HIGH);
			p3->setValue(LOW);
			delay(t);
			p1->setValue(HIGH);
			p2->setValue(LOW);
			delay(t);
		}
		p1->setValue(LOW);
	}
	*/

		string in = "";
		int pct, pin;
		while (in != "quit") {

			while (in != "W" && in != "w" && in != "P" && in != "p" && in != "Q" && in != "q" && in != "i" && in != "I") {
				cout << "(W)heels, (P)ower, P(i)n control, (R)ead sensor, (Q)uit : ";
				cin >> in;
			}

			if (in == "r" || in == "R") {
				cout << "Not implemented yet.";
			}

			if (in == "p" || in == "P") {
				pct = 0;
				cout << "Set % throttle to: ";
				cin >> in;
				try { pct = stoi(in); }
				catch (const invalid_argument& e) { }
				if ((pct >=- 100) && (pct <= 100)) car.speedPct(pct);
			}

			if (in == "w" || in == "W") {
				pct = 0;
				cout << "Set % turn to: ";
				cin >> in;
				try { pct = stoi(in); }
				catch (const invalid_argument& e) { }
				if ((pct >= -100) && (pct <= 100)) { car.turnPct(pct); }
			}

			if (in == "q" || in == "Q") {
				in = "quit";
			}

			if (in == "i" || in == "I") {
				pct = 0;
				pin = -1;
				cout << "Which pin: ";
				cin >> in;
				try { pin = stoi(in); }
				catch (const invalid_argument& e) { }
				if (pin != -1) {
					cout << "Value: ";
					cin >> in;
					try { pct = stoi(in); }
					catch (const invalid_argument& e) { }
					car.getRaspberryPi()->getGpio()->getPin(pin)->setValue(pct);
				}
				in = "";
			}

		}
		car.stop();
	}

}



