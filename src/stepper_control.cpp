#include <Arduino.h>

#include "common.h"

#include "Commands.hpp"
#include "stepper_control.hpp"

// init our variables
long max_delta;
long x_counter;
long y_counter;
long z_counter;
bool x_can_step;
bool y_can_step;
bool z_can_step;
int milli_delay;

void init_steppers()
{
	// turn them off to start.
	disable_steppers();

	// init our points.
	Commands::current_units.x = 0.0;
	Commands::current_units.y = 0.0;
	Commands::current_units.z = 0.0;
	Commands::target_units.x = 0.0;
	Commands::target_units.y = 0.0;
	Commands::target_units.z = 0.0;

	pinMode(WIRING_X_STEP_PIN, OUTPUT);
	pinMode(WIRING_X_DIRECTION_PIN, OUTPUT);
	pinMode(WIRING_X_ENABLE_PIN, OUTPUT);
	pinMode(WIRING_X_LIMIT_MIN_PIN, INPUT_PULLUP);
	pinMode(WIRING_X_LIMIT_MAX_PIN, INPUT_PULLUP);

	pinMode(WIRING_Y_STEP_PIN, OUTPUT);
	pinMode(WIRING_Y_DIRECTION_PIN, OUTPUT);
	pinMode(WIRING_Y_ENABLE_PIN, OUTPUT);
	pinMode(WIRING_Y_LIMIT_MIN_PIN, INPUT_PULLUP);
	pinMode(WIRING_Y_LIMIT_MAX_PIN, INPUT_PULLUP);

	pinMode(WIRING_Z_STEP_PIN, OUTPUT);
	pinMode(WIRING_Z_DIRECTION_PIN, OUTPUT);
	pinMode(WIRING_Z_ENABLE_PIN, OUTPUT);
	pinMode(WIRING_Z_LIMIT_MIN_PIN, INPUT_PULLUP);
	pinMode(WIRING_Z_LIMIT_MAX_PIN, INPUT_PULLUP);

	// figure our stuff.
	calculate_deltas();
	goto_machine_zero();
}

void goto_machine_zero()
{
	Serial.println("init");
	move_to_max(WIRING_X_LIMIT_MIN_PIN, WIRING_X_STEP_PIN, WIRING_X_DIRECTION_PIN, 0);
	move_to_max(WIRING_Y_LIMIT_MIN_PIN, WIRING_Y_STEP_PIN, WIRING_Y_DIRECTION_PIN, 0);
	Serial.println("ok");
}

void move_to_max(int limiter_pin, int stepper_pin, int stepper_dir_pin, int dir)
{
	/* Moves to the maximum possible position
	 */
	while (can_step(limiter_pin, limiter_pin, 0, 1, dir))
	{
		do_step(stepper_pin, stepper_dir_pin, 0);
		delay(1);
	}
	// slowly back unitl pin is released
	while (!can_step(limiter_pin, limiter_pin, 0, 1, dir))
	{
		do_step(stepper_pin, stepper_dir_pin, 1);
		delay(100);
	}
}

void dda_move(long micro_delay)
{
	// enable our steppers
	digitalWrite(WIRING_X_ENABLE_PIN, HIGH);
	digitalWrite(WIRING_Y_ENABLE_PIN, HIGH);
	digitalWrite(WIRING_Z_ENABLE_PIN, HIGH);

	// figure out our deltas
	max_delta = max(Commands::delta_steps.x, Commands::delta_steps.y);
	max_delta = max(Commands::delta_steps.z, max_delta);

	// init stuff.
	long x_counter = -max_delta / 2;
	long y_counter = -max_delta / 2;
	long z_counter = -max_delta / 2;

	// our step flags
	bool x_can_step = 0;
	bool y_can_step = 0;
	bool z_can_step = 0;

	if (micro_delay >= 16383)
		milli_delay = micro_delay / 1000;
	else
		milli_delay = 0;

	// do our DDA line!
	do
	{
		x_can_step = can_step(WIRING_X_LIMIT_MIN_PIN, WIRING_X_LIMIT_MAX_PIN, Commands::current_steps.x, Commands::target_steps.x, Commands::x_direction);
		y_can_step = can_step(WIRING_Y_LIMIT_MIN_PIN, WIRING_Y_LIMIT_MAX_PIN, Commands::current_steps.y, Commands::target_steps.y, Commands::y_direction);
		z_can_step = can_step(WIRING_Z_LIMIT_MIN_PIN, WIRING_Z_LIMIT_MAX_PIN, Commands::current_steps.z, Commands::target_steps.z, Commands::z_direction);

		if (x_can_step)
		{
			x_counter += Commands::delta_steps.x;

			if (x_counter > 0)
			{
				do_step(WIRING_X_STEP_PIN, WIRING_X_DIRECTION_PIN, Commands::x_direction);
				x_counter -= max_delta;

				if (Commands::x_direction)
					Commands::current_steps.x++;
				else
					Commands::current_steps.x--;
			}
		}

		if (y_can_step)
		{
			y_counter += Commands::delta_steps.y;

			if (y_counter > 0)
			{
				do_step(WIRING_Y_STEP_PIN, WIRING_Y_DIRECTION_PIN, Commands::y_direction);
				y_counter -= max_delta;

				if (Commands::y_direction)
					Commands::current_steps.y++;
				else
					Commands::current_steps.y--;
			}
		}

		if (z_can_step)
		{
			z_counter += Commands::delta_steps.z;

			if (z_counter > 0)
			{
				if (WIRING_Z_AXIS_SUPPORTED == 0)
				{
					do_step(WIRING_Z_STEP_PIN, WIRING_Z_DIRECTION_PIN, Commands::z_direction);
				}
				z_counter -= max_delta;

				if (Commands::z_direction)
					Commands::current_steps.z++;
				else
					Commands::current_steps.z--;
			}
		}

		// wait for next step.
		if (milli_delay > 0)
			delay(milli_delay);
		else
			delayMicroseconds(micro_delay);
	} while (x_can_step || y_can_step || z_can_step);

	// set our points to be the same
	Commands::current_units.x = Commands::target_units.x;
	Commands::current_units.y = Commands::target_units.y;
	Commands::current_units.z = Commands::target_units.z;
	calculate_deltas();
}

bool can_step(byte min_pin, byte max_pin, long current, long target, byte direction)
{
	// stop us if we're on target
	if (target == current)
		return false;
	// stop us if we're at home and still going
	else if (read_switch(min_pin) && !direction)
		return false;
	// stop us if we're at max and still going
	else if (read_switch(max_pin) && direction)
		return false;

	// default to being able to step
	return true;
}

void do_step(byte pinA, byte pinB, byte dir)
{
	switch (dir << 2 | digitalRead(pinA) << 1 | digitalRead(pinB))
	{
	case 0: /* 0 00 -> 10 */
	case 5: /* 1 01 -> 11 */
		digitalWrite(pinA, HIGH);
		break;
	case 1: /* 0 01 -> 00 */
	case 7: /* 1 11 -> 10 */
		digitalWrite(pinB, LOW);
		break;
	case 2: /* 0 10 -> 11 */
	case 4: /* 1 00 -> 01 */
		digitalWrite(pinB, HIGH);
		break;
	case 3: /* 0 11 -> 01 */
	case 6: /* 1 10 -> 00 */
		digitalWrite(pinA, LOW);
		break;
	}
	delayMicroseconds(5);
}

bool read_switch(byte pin)
{
	// dual read as crude debounce

	if (PARAMETERS_LIMIT_SWITCHES_ACTIVE_LOW)
		return !digitalRead(pin) && !digitalRead(pin);
	else
		return digitalRead(pin) && digitalRead(pin);
}

long to_steps(float steps_per_unit, float units)
{
	return steps_per_unit * units;
}

void set_target(float x, float y, float z)
{
	Commands::target_units.x = x;
	Commands::target_units.y = y;
	Commands::target_units.z = z;

	calculate_deltas();
}

void set_position(float x, float y, float z)
{
	Commands::current_units.x = x;
	Commands::current_units.y = y;
	Commands::current_units.z = z;

	calculate_deltas();
}

void calculate_deltas()
{
	// figure our deltas.
	Commands::delta_units.x = abs(Commands::target_units.x - Commands::current_units.x);
	Commands::delta_units.y = abs(Commands::target_units.y - Commands::current_units.y);
	Commands::delta_units.z = abs(Commands::target_units.z - Commands::current_units.z);

	// set our steps current, target, and delta
	Commands::current_steps.x = to_steps(Commands::x_units, Commands::current_units.x);
	Commands::current_steps.y = to_steps(Commands::y_units, Commands::current_units.y);
	Commands::current_steps.z = to_steps(Commands::z_units, Commands::current_units.z);

	Commands::target_steps.x = to_steps(Commands::x_units, Commands::target_units.x);
	Commands::target_steps.y = to_steps(Commands::y_units, Commands::target_units.y);
	Commands::target_steps.z = to_steps(Commands::z_units, Commands::target_units.z);

	Commands::delta_steps.x = abs(Commands::target_steps.x - Commands::current_steps.x);
	Commands::delta_steps.y = abs(Commands::target_steps.y - Commands::current_steps.y);
	Commands::delta_steps.z = abs(Commands::target_steps.z - Commands::current_steps.z);

	// what is our direction
	Commands::x_direction = (Commands::target_units.x >= Commands::current_units.x);
	Commands::y_direction = (Commands::target_units.y >= Commands::current_units.y);
	Commands::z_direction = (Commands::target_units.z >= Commands::current_units.z);

	// set our direction pins as well
	digitalWrite(WIRING_X_DIRECTION_PIN, Commands::x_direction);
	digitalWrite(WIRING_Y_DIRECTION_PIN, Commands::y_direction);
	digitalWrite(WIRING_Z_DIRECTION_PIN, Commands::z_direction);
}

long calculate_feedrate_delay(float feedrate)
{
	// how long is our line length?
	float distance = sqrt(Commands::delta_units.x * Commands::delta_units.x + Commands::delta_units.y * Commands::delta_units.y + Commands::delta_units.z * Commands::delta_units.z);
	long master_steps = 0;

	// find the dominant axis.
	if (Commands::delta_steps.x > Commands::delta_steps.y)
	{
		if (Commands::delta_steps.z > Commands::delta_steps.x)
			master_steps = Commands::delta_steps.z;
		else
			master_steps = Commands::delta_steps.x;
	}
	else
	{
		if (Commands::delta_steps.z > Commands::delta_steps.y)
			master_steps = Commands::delta_steps.z;
		else
			master_steps = Commands::delta_steps.y;
	}

	// calculate delay between steps in microseconds.  this is sort of tricky, but not too bad.
	// the formula has been condensed to save space.  here it is in english:
	//  distance / feedrate * 60000000.0 = move duration in microseconds
	//  move duration / master_steps = time between steps for master axis.

	return ((distance * 60000000.0) / feedrate) / master_steps;
}

long getMaxSpeed()
{
	if (Commands::delta_steps.z > 0)
		return calculate_feedrate_delay(PARAMETERS_MAX_Z_RATE);
	else
		return calculate_feedrate_delay(PARAMETERS_MAX_XY_RATE);
}

void disable_steppers()
{
	// enable our steppers
	digitalWrite(WIRING_X_ENABLE_PIN, LOW);
	digitalWrite(WIRING_Y_ENABLE_PIN, LOW);
	digitalWrite(WIRING_Z_ENABLE_PIN, LOW);
}
