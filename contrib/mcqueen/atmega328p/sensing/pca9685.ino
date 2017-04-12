
#include <Wire.h>

#define ADDRESS        0x40
#define FREQUENCY      50          /Hz
#define OSC_CLOCK      2500000
#define RESOLUTION     4096
#define REG_PRESCALE   0xFE
#define REG_MODE1      0x00
#define MODE1_RESTART  0x80
#define MODE1_EXTCLK   0x40
#define MODE1_AI       0x20
#define MODE1_SLEEP    0x10
#define MODE1_SUB1     0x08
#define MODE1_SUB2     0x04
#define MODE1_SUB3     0x02
#define MODE1_ALLCALL  0x01

void init() {
  Wire.begin()
}

void sleep() {
  Wire.beginTransmission(ADDRESS);
  Wire.write(REG_MODE1);
  Wire.write(MODE1_SLEEP);
  Wire.endTransmission();
}

void wakeUp() {
  Wire.beginTransmission(ADDRESS);
  Wire.write(REG_MODE1);
  Wire.write(MODE1_AI);
  Wire.endTransmission();
}


