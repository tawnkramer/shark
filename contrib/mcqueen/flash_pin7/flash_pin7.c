
#include <stdio.h>
#include <wiringPi.h>

int main(int argc, char **argv)
{
	int c;
	if (wiringPiSetup() == -1) {
		printf("\nCan't initialize wiringPi library. Make sure you use as root.\n");
		return 1;
	}
	
	pinMode(7, OUTPUT);
	for (c = 0; c < 20; c++) {
		digitalWrite(7, HIGH);
		delay(1000);
		digitalWrite(7, LOW);
		delay(1000);
	}
	
	return 0;
}

