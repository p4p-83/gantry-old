#include <Arduino.h>

#include "common.h"
#include "process_string.hpp"
#include "stepper_control.hpp"

/****************************************************************
 * digital i/o pin assignment
 *
 * this uses the undocumented feature of Arduino - pins 14-19 correspond to analog 0-5
 ****************************************************************/
// A Ports are A for Analog, not a hex code
// Pin numbers from diagram on http://learn.makeblock.com/makeblock-orion/
// Port 1 Uno/Orion
int X_DIR_PIN = 10;
int X_STEP_PIN = 11;

// Port 2 Uno/Orion
int Y_DIR_PIN = 3;
int Y_STEP_PIN = 9;

// Port 3 Uno/Orion
int Y_MIN_PIN = 17;
int Y_MAX_PIN = 16;

// Port 6 Uno/Orion
// Note instructions say port 8
// but the X Limits dont work with pins A0 and A7, they only work in port 6
int X_MIN_PIN = 13; // (16, 17 can be A2, A3)
int X_MAX_PIN = 12;

// Port 7 Uno/Orion
// Z_DIR_PIN Not used for servo.
int Z_DIR_PIN = -1;
int Z_STEP_PIN = A1; // 15

// Nothing connected here, just defining to a value.
int Z_MIN_PIN = -1;
int Z_MAX_PIN = -1;

int X_ENABLE_PIN = -1;
int Y_ENABLE_PIN = -1;
int Z_ENABLE_PIN = -1;
int Z_ENABLE_SERVO = 1;

// define the parameters of our machine.
float X_STEPS_PER_INCH = 48;
float X_STEPS_PER_MM = 40;
int X_MOTOR_STEPS = 100;

float Y_STEPS_PER_INCH = 48;
float Y_STEPS_PER_MM = 40;
int Y_MOTOR_STEPS = 100;

float Z_STEPS_PER_INCH = 48;
float Z_STEPS_PER_MM = 40;
int Z_MOTOR_STEPS = 100;

// our maximum feedrates
long FAST_XY_FEEDRATE = 2000;
long FAST_Z_FEEDRATE = 2000;

int SENSORS_INVERTING = 1;

char commands[COMMAND_SIZE];
uint8_t serial_count;

Servo servo;

int currentPosServo = 90;
int targetPosServo = 90;
bool comment = false;

int no_data = 0;

void setup()
{
	// Do startup stuff here
	Serial.begin(115200);
	if (Z_ENABLE_SERVO == 1)
	{
		servo.attach(Z_STEP_PIN);
	}
	// other initialization.
	init_process_string();
	init_steppers();
	process_string("G90", 3); // Absolute Position
	Serial.println("start");
}

void loop()
{

	char c;
	// read in characters if we got them.
	if (Serial.available() > 0)
	{
		c = Serial.read();
		no_data = 0;
		// newlines are ends of commands.
		if (c != '\n')
		{
			if (c == 0x18)
			{
				Serial.println("Grbl 1.0");
			}
			else
			{
				if (c == '(')
				{
					comment = true;
				}
				// If we're not in comment mode, add it to our array.
				if (!comment)
				{
					commands[serial_count] = c;
					serial_count++;
				}
				if (c == ')')
				{
					comment = false; // End of comment - start listening again
				}
			}
		}
	}
	else
	{
		no_data++;
		delayMicroseconds(100);

		// if theres a pause or we got a real command, do it
		if (serial_count && (c == '\n' || no_data > 100))
		{
			// process our command!
			process_string(commands, serial_count);
			// clear command.
			init_process_string();
		}

		// no data?  turn off steppers
		if (no_data > 1000)
		{
			disable_steppers();
		}
	}
	//        return;
	//                delay(5);
	//                int dPos = abs(currentPosServo-targetPosServo);
	//                if(currentPosServo<targetPosServo){
	//                   currentPosServo += dPos>8?6:1;
	//                }else if(currentPosServo>targetPosServo){
	//                   currentPosServo -= dPos>8?6:1;
	//                }
}
