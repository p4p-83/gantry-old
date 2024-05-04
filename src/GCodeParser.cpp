#include <Arduino.h>

#include "common.h"
#include "process_string.hpp"
#include "stepper_control.hpp"

void setup()
{
	Serial.begin(115200);

	init_process_string();
	init_steppers();
	process_string("G90", 3); // Absolute Position
	Serial.println("start");
}

void loop()
{
	static bool comment = false;
	static int no_data = 0;

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
}
