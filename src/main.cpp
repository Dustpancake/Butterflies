#include "Arduino.h"
#include "Servo.h"
#include <math.h>

/*
 *  
 *  DEFAULT CONFIGURATION IS FOR 3 LEDS AND 3 SERVOS
 *  Data sent over must be in chunks of 6 bytes:
 *    The first 3 are for LEDS.
 *    The last 3 are the SERVOS.
 *  
 */

#define ARRAY_SIZE(x) ((sizeof x) / (sizeof *x))

// PIN SETUP
const int led_pins[]    = { 11 , 10 , 9 };
const int servo_pins[]  = { 6  , 5  , 3 };

// SERVO ANGLE SETUP
#define MAX_POS 180
#define MIN_POS 80

// CONSTANTS
#define SERV_SIZE ARRAY_SIZE(servo_pins)
#define LEDS_SIZE ARRAY_SIZE(led_pins)

// SERVO CONFIG
int pos[SERV_SIZE];
bool reverse[SERV_SIZE];
byte speeds[SERV_SIZE];
Servo servos[SERV_SIZE];

// LED CONFIG
byte brightness[LEDS_SIZE];

// DECLARATIONS --------------------------------------------- //

void init_arrays();
void flush_buffer();
void adjust_pos(int i);
void step_servo();
void read_data(byte* ptr);
void write_data(byte* ptr, int length);
void fade_leds();

// ENTRY POINTS --------------------------------------------- //

void setup() {
  // init everything
  Serial.begin(9600);
  init_arrays();

  // configure pins
  for (int i = 0; i < SERV_SIZE; i++) {
    servos[i].attach(servo_pins[i]);  
    servos[i].write(pos[i]);
  }
  for (int i = 0; i < LEDS_SIZE; i++) {
    pinMode(led_pins[i], OUTPUT);
  }

  // announce program
  Serial.println(F("running-butterfly-a1.0"));
}

void loop() {
  step_servo();
  fade_leds();
}

// CALLBACKS ------------------------------------------------ //

void serialEvent() {
  /*  
   * read in the serial data and apply mathematical conversions
   * output the brightnesses and speeds for debugging purposes
   */

  byte data[SERV_SIZE + LEDS_SIZE];
  read_data(data);
  for (int i = 0; i < LEDS_SIZE; i++) {
    brightness[i] = data[i];
  }
  for (int i = 0; i < SERV_SIZE; i++) {
    speeds[i] = (log((float) data[i + LEDS_SIZE]));
  }
  write_data(brightness, LEDS_SIZE);
  write_data(speeds, SERV_SIZE);
  flush_buffer();
}

// DEFINITIONS ---------------------------------------------- //

void flush_buffer()
{
  while (Serial.read() != '\r')
   ; // do nothing
}

void fade_leds() {
  for (int i = 0; i < LEDS_SIZE; i++) {
    analogWrite(led_pins[i], brightness[i]);
  }
}

void adjust_pos(int i) {
  // change positions
  if (reverse[i]) {
    pos[i] -= speeds[i];
  } else {
    pos[i] += speeds[i];
  }
  // check bounds
  if (pos[i] <= MIN_POS) {
    reverse[i] = !reverse[i];
    pos[i] = MIN_POS;
  } else if (pos[i] >= MAX_POS) {
    reverse[i] = !reverse[i];
    pos[i] = MAX_POS;
  }
  servos[i].write(pos[i]);
}

void step_servo() {
  // adjust positions of all servos
  for (int i = 0; i < SERV_SIZE; i++) {
    adjust_pos(i);
  }
  // and delay
  delay(20);
}

void read_data(byte* ptr) {
  while (Serial.available() < SERV_SIZE + LEDS_SIZE) {
    ; // wait until 8 bytes available
  }
  for (int i = 0; i < SERV_SIZE + LEDS_SIZE; i++) {
    ptr[i] = (byte)(Serial.read());
  }
}

void write_data(byte* ptr, int length) {
  for (int i = 0; i < length; i++) {
    Serial.write((int)(ptr[i]));
  }
}

void init_arrays() {
  for (int i = 0; i < SERV_SIZE; i++) {
    // set speeds and positions to 0
    speeds[i] = 0;
    pos[i] = 0;
    reverse[0] = false;
  }
  for (int i = 0; i < LEDS_SIZE; i++) {
    // set led brightnesses to 0
    brightness[i]   = 0;
  }
}