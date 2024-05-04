#ifndef COMMON_H_
#define COMMON_H_

#include <Servo.h>

#include <stdint.h>

/****************************************************************
 * digital i/o pin assignment
 *
 * this uses the undocumented feature of Arduino - pins 14-19 correspond to analog 0-5
 ****************************************************************/
// A Ports are A for Analog, not a hex code
// Pin numbers from diagram on http://learn.makeblock.com/makeblock-orion/
// Port 1 Uno/Orion
extern int X_DIR_PIN;
extern int X_STEP_PIN;

// Port 2 Uno/Orion
extern int Y_DIR_PIN;
extern int Y_STEP_PIN;

// Port 3 Uno/Orion
extern int Y_MIN_PIN;
extern int Y_MAX_PIN;

// Port 6 Uno/Orion
// Note instructions say port 8
// but the X Limits dont work with pins A0 and A7, they only work in port 6
extern int X_MIN_PIN; // (16, 17 can be A2, A3)
extern int X_MAX_PIN;

// Port 7 Uno/Orion
// Z_DIR_PIN Not used for servo.
extern int Z_DIR_PIN;
extern int Z_STEP_PIN; // 15

// Nothing connected here, just defining to a value.
extern int Z_MIN_PIN;
extern int Z_MAX_PIN;

extern int X_ENABLE_PIN;
extern int Y_ENABLE_PIN;
extern int Z_ENABLE_PIN;
extern int Z_ENABLE_SERVO;

// define the parameters of our machine.
extern float X_STEPS_PER_INCH;
extern float X_STEPS_PER_MM;
extern int X_MOTOR_STEPS;

extern float Y_STEPS_PER_INCH;
extern float Y_STEPS_PER_MM;
extern int Y_MOTOR_STEPS;

extern float Z_STEPS_PER_INCH;
extern float Z_STEPS_PER_MM;
extern int Z_MOTOR_STEPS;

// our maximum feedrates
extern long FAST_XY_FEEDRATE;
extern long FAST_Z_FEEDRATE;

// Units in curve section
#define CURVE_SECTION_INCHES 0.019685
#define CURVE_SECTION_MM	 0.5

// Set to one if sensor outputs inverting (ie: 1 means open, 0 means closed)
// RepRap opto endstops are *not* inverting.
extern int SENSORS_INVERTING;

#define COMMAND_SIZE 128
extern char commands[COMMAND_SIZE];
extern uint8_t serial_count;

extern Servo servo;

extern int currentPosServo;
extern int targetPosServo;
extern bool comment;

#endif
