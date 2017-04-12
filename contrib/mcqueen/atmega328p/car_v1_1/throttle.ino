#include <Servo.h>

#define THROTTLE_MAXREVERSE  1000
#define THROTTLE_MAXFORWARD  2000
#define THROTTLE_IDLE        1500
#define THROTTLEPIN          10

Servo throttle;
int throttle_pulse;

void throttle_init () {
  throttle.attach (THROTTLEPIN);

  // ESC init sequence
  throttle_set ( THROTTLE_MAXFORWARD );
  delay (25);
  throttle_set ( THROTTLE_MAXREVERSE );
  delay (25);
  throttle_set ( THROTTLE_IDLE );

  throttle_pulse = THROTTLE_IDLE;
}

void throttle_set (int pulse) {
  throttle.writeMicroseconds (pulse);
  throttle_pulse = pulse;
}

void throttle_forwardPct (int pct) {
  int newPulse = int( double(THROTTLE_IDLE + ( double(abs(THROTTLE_IDLE - THROTTLE_MAXFORWARD))*(double(pct)/100) )));

  throttle.writeMicroseconds ( newPulse );
  throttle_pulse = newPulse;
}

void throttle_reversePct (int pct) {
  int newPulse = int( double(THROTTLE_IDLE - ( double(abs(THROTTLE_IDLE - THROTTLE_MAXREVERSE))*(double(pct)/100) )));

  if (throttle_pulse >= THROTTLE_IDLE) {
    throttle_idle ();
    delay (100);
    throttle.writeMicroseconds ( newPulse );
    delay (100);
    throttle_idle ();
    delay (100);
  }
  throttle.writeMicroseconds ( newPulse );
  throttle_pulse = newPulse;
}

void throttle_idle () {
  throttle.writeMicroseconds ( THROTTLE_IDLE );
  throttle_pulse = THROTTLE_IDLE;
}

void throttle_stop () {
  /*
  int i;
  if (throttle_pulse > THROTTLE_IDLE) {


    for (i=throttle_pulse; i > THROTTLE_IDLE; i-=10) {
      throttle_set (i);
      delay (20);
    }
  }
  else if (throttle_pulse < THROTTLE_IDLE) {
    for (i=throttle_pulse; i < THROTTLE_IDLE; i+=10) {
      throttle_set (i);
      delay (20);
    }
  }
  */
  throttle.writeMicroseconds ( THROTTLE_IDLE );
  throttle_pulse = THROTTLE_IDLE;
}

