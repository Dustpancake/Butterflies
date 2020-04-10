/**
  Created by Fergus Baker; April 2020.
  */

#include "Arduino.h"
#include "Servo.h"

/*
 *  
 *  DEFAULT CONFIGURATION IS FOR 3 LEDS AND 3 SERVOS
 *  Data sent over must be in chunks of 6 bytes:
 *    The first 3 are for LEDS.
 *    The last 3 are the SERVOS.
 *  
 */

#define ARRAY_SIZE(x) ((sizeof x) / (sizeof *x))
using byte = unsigned char;

// PIN SETUP
const int led_pins[]    = { 3 , 5 , 6 , 9 , 10 , 11 };
const int servo_pins[]  = { };

// SERVO ANGLE SETUP
#define MAX_POS 180
#define MIN_POS 80

// CONSTANTS
#define SERV_SIZE 0
#define LEDS_SIZE 6

// SERVO CONFIG
int pos[SERV_SIZE];
bool reverse[SERV_SIZE];
byte* speeds;
Servo servos[SERV_SIZE];

// LED CONFIG
byte* brightness;

// SERVO SPEED DATA
// ln((i**2)/200
const byte SPEEDS[256] = {
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2,
      2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3,
      3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
      3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4,
      4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
      4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
      4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5,
      5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
      5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
      5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
      5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5
};

// OTHER GLOBALS
bool newData = false;

// DECLARATIONS --------------------------------------------- //

void init_arrays();
void flush_buffer();
void adjust_pos(int i);
void step_servo();
void read_data(byte* ptr);
void write_data();
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
  // step_servo();
}

// CALLBACKS ------------------------------------------------ //

void serialEvent() {
  /*  
   * read in the serial data and apply mathematical conversions
   * output the brightnesses and speeds for debugging purposes
   */

  byte data[SERV_SIZE + LEDS_SIZE];
  read_data(data);

  brightness = data;
  speeds = data + ( LEDS_SIZE * sizeof(byte) );

  write_data();
  flush_buffer();
  
  // update LEDs
  fade_leds();
}

// DEFINITIONS ---------------------------------------------- //

void flush_buffer()
{
  while (Serial.read() != '\r')
   loop(); // do nothing
}

void fade_leds() {
  for (int i = 0; i < LEDS_SIZE; i++) {
    analogWrite(led_pins[i], brightness[i]);
  }
}

void adjust_pos(int i) {
  // change positions
  if (reverse[i]) {
    pos[i] -= SPEEDS[speeds[i]];
  } else {
    pos[i] += SPEEDS[speeds[i]];
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
    loop(); // wait until 6 bytes available
  }
  for (int i = 0; i < SERV_SIZE + LEDS_SIZE; i++) {
    ptr[i] = (byte)(Serial.read());
  }
}

void write_data() {
  for (int i = 0; i < LEDS_SIZE; i++) {
    Serial.write((byte) brightness[i]);
  }
  for (int i = 0; i < SERV_SIZE; i++) {
    Serial.write((byte) speeds[i]);
  }
}

void init_arrays() {
  brightness = (byte*) malloc(LEDS_SIZE);
  speeds = (byte*) malloc(SERV_SIZE);

  for (int i = 0; i < SERV_SIZE; i++) {
    // set speeds and positions to 0
    speeds[i] = 0;
    pos[i] = 0;
    reverse[0] = false;
  }
  for (int i = 0; i < LEDS_SIZE; i++) {
    // set led brightnesses to 0
    brightness[i] = 0;
  }
}