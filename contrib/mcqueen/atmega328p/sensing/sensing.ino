/**
 *
 * Sensing program for atmega328 reading commands from the serial port and returning value of the sensors
 *
 **/

/**
 * -----------------------------------------------------------------------------------------------
 * Global variables declaration
 * -----------------------------------------------------------------------------------------------
 **/
char inCommand [255] = {};   // string to receive commands on the serial line
int inCommandPos = 0;
char reply [4096] = {};

int ledRX = 5;
int ledTX = 6;



/**
 * -----------------------------------------------------------------------------------------------
 * Setup Routine
 * -----------------------------------------------------------------------------------------------
 **/
void setup() {
  Serial.begin(9600);
  pinMode(ledRX, OUTPUT);
  pinMode(ledTX, OUTPUT);
}




/**
 * -----------------------------------------------------------------------------------------------
 * Main program loop
 * -----------------------------------------------------------------------------------------------
 **/
void loop() {
  int bytesToRead = 0;
  byte incomingByte = 0;
  inCommandPos = 0;

  // Turn off leds
  digitalWrite (ledRX, LOW);
  digitalWrite (ledTX, LOW);

  // Read sensors

  // Check for serial request
  bytesToRead = Serial.available();
  if (bytesToRead > 0) {
    incomingByte = Serial.read();
    digitalWrite(ledRX, HIGH);
    delay(25);

    // reading bytes until end of command is received (";")
    inCommand[inCommandPos++]=incomingByte;
    
  }
}



/** List of IN-COMMANDS RECOGNIZED BY THE SYSTEM:
    
    !!! Important Notes
    - All commands are case sensitive


**/


