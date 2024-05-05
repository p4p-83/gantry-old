#include <Arduino.h>

#include "common.h"

#include "Commands.hpp"
#include "Steppers.hpp"

// our direction vars
Steppers::Direction Steppers::x_direction = Steppers::Direction::TOWARDS_MAX;
Steppers::Direction Steppers::y_direction = Steppers::Direction::TOWARDS_MAX;
Steppers::Direction Steppers::z_direction = Steppers::Direction::TOWARDS_MAX;

/**
 * @brief Convert a unit measurement into step counts.
 *
 * @param stepsPerUnit The number of steps per unit value.
 * @param units The unit measurement.
 * @return long The number of steps.
 */
static uint32_t UnitsToSteps( float stepsPerUnit, float units );

/**
 * @brief Read the state of a limit switch pin.
 *
 * @param pin The pin that the limit switch is connected to.
 * @return true if the limit switch is depressed. \
 * @return false otherwise.
 */
static bool ReadLimitSwitch( uint8_t pin );

/**
 * @brief Perform a step in the specified direction.
 *
 * @param stepperPin The stepper pin.
 * @param stepperDirectionPin The stepper direction pin.
 * @param direction The direction in which to step.
 */
static void DoStep( uint8_t stepperPin, uint8_t stepperDirectionPin, Steppers::Direction direction );

void Steppers::Initialise( void )
{
	// Disable motors to begin with
	Steppers::Disable();

	// init our points.
	// TODO: nuke these out of existence
	Commands::current_units.x = 0.0;
	Commands::current_units.y = 0.0;
	Commands::current_units.z = 0.0;
	Commands::target_units.x = 0.0;
	Commands::target_units.y = 0.0;
	Commands::target_units.z = 0.0;

	Serial.println( "Setting pinMode configurations" );

	pinMode( WIRING_X_STEP_PIN, OUTPUT );
	pinMode( WIRING_X_DIRECTION_PIN, OUTPUT );
	pinMode( WIRING_X_ENABLE_PIN, OUTPUT );
	pinMode( WIRING_X_LIMIT_MIN_PIN, INPUT_PULLUP );
	pinMode( WIRING_X_LIMIT_MAX_PIN, INPUT_PULLUP );

	pinMode( WIRING_Y_STEP_PIN, OUTPUT );
	pinMode( WIRING_Y_DIRECTION_PIN, OUTPUT );
	pinMode( WIRING_Y_ENABLE_PIN, OUTPUT );
	pinMode( WIRING_Y_LIMIT_MIN_PIN, INPUT_PULLUP );
	pinMode( WIRING_Y_LIMIT_MAX_PIN, INPUT_PULLUP );

	pinMode( WIRING_Z_STEP_PIN, OUTPUT );
	pinMode( WIRING_Z_DIRECTION_PIN, OUTPUT );
	pinMode( WIRING_Z_ENABLE_PIN, OUTPUT );
	pinMode( WIRING_Z_LIMIT_MIN_PIN, INPUT_PULLUP );
	pinMode( WIRING_Z_LIMIT_MAX_PIN, INPUT_PULLUP );

	Serial.println( "Calculating deltas" );
	Steppers::CalculateDeltas();

	Steppers::MoveToZero();
}

void Steppers::Disable( void )
{
	Serial.println( "Disabling stepper motors" );
	digitalWrite( WIRING_X_ENABLE_PIN, LOW );
	digitalWrite( WIRING_Y_ENABLE_PIN, LOW );
	digitalWrite( WIRING_Z_ENABLE_PIN, LOW );
}

void Steppers::Enable( void )
{
	Serial.println( "Enabling stepper motors" );
	digitalWrite( WIRING_X_ENABLE_PIN, HIGH );
	digitalWrite( WIRING_Y_ENABLE_PIN, HIGH );
	digitalWrite( WIRING_Z_ENABLE_PIN, HIGH );
}

// TODO: I want to use this even for MoveToLimit
uint32_t Steppers::GetMinRateDelayMicroseconds( void )
{
	// TODO: Nasty. Pass as arg /please/.
	if ( Commands::delta_steps.z > 0 )
	{
		return Steppers::CalculateRateDelayMicroseconds( PARAMETERS_MAX_Z_MM_PER_SEC );
	}
	else
	{
		return Steppers::CalculateRateDelayMicroseconds( PARAMETERS_MAX_XY_MM_PER_SEC );
	}
}

// TODO: rename all units -> mm
void Steppers::SetPosition( float xUnits, float yUnits, float zUnits )
{
	Commands::current_units.x = xUnits;
	Commands::current_units.y = yUnits;
	Commands::current_units.z = zUnits;

	Steppers::CalculateDeltas();
}

void Steppers::SetTarget( float xUnits, float yUnits, float zUnits )
{
	Commands::target_units.x = xUnits;
	Commands::target_units.y = yUnits;
	Commands::target_units.z = zUnits;

	Steppers::CalculateDeltas();
}

void Steppers::CalculateDeltas( void )
{
	Serial.println( "Calculating deltas" );

	// Calculate the delta units between the current and target position
	Commands::delta_units.x = abs( Commands::target_units.x - Commands::current_units.x );
	Commands::delta_units.y = abs( Commands::target_units.y - Commands::current_units.y );
	Commands::delta_units.z = abs( Commands::target_units.z - Commands::current_units.z );

	// Convert current units to step counts
	Commands::current_steps.x = UnitsToSteps( Commands::x_units, Commands::current_units.x );
	Commands::current_steps.y = UnitsToSteps( Commands::y_units, Commands::current_units.y );
	Commands::current_steps.z = UnitsToSteps( Commands::z_units, Commands::current_units.z );

	// Convert target units to step counts
	Commands::target_steps.x = UnitsToSteps( Commands::x_units, Commands::target_units.x );
	Commands::target_steps.y = UnitsToSteps( Commands::y_units, Commands::target_units.y );
	Commands::target_steps.z = UnitsToSteps( Commands::z_units, Commands::target_units.z );

	// Calculate the delta steps between the current and target position
	Commands::delta_steps.x = abs( Commands::target_steps.x - Commands::current_steps.x );
	Commands::delta_steps.y = abs( Commands::target_steps.y - Commands::current_steps.y );
	Commands::delta_steps.z = abs( Commands::target_steps.z - Commands::current_steps.z );

	// Determine the required direction of travel
	Steppers::x_direction = ( Commands::target_units.x >= Commands::current_units.x )
		? Steppers::Direction::TOWARDS_MAX
		: Steppers::Direction::TOWARDS_MIN;
	Steppers::y_direction = ( Commands::target_units.y >= Commands::current_units.y )
		? Steppers::Direction::TOWARDS_MAX
		: Steppers::Direction::TOWARDS_MIN;
	Steppers::z_direction = ( Commands::target_units.z >= Commands::current_units.z )
		? Steppers::Direction::TOWARDS_MAX
		: Steppers::Direction::TOWARDS_MIN;

	// Set direction pins
	digitalWrite( WIRING_X_DIRECTION_PIN, ( ( Steppers::x_direction == Steppers::Direction::TOWARDS_MAX ) ? HIGH : LOW ) );
	digitalWrite( WIRING_Y_DIRECTION_PIN, ( ( Steppers::y_direction == Steppers::Direction::TOWARDS_MAX ) ? HIGH : LOW ) );
	digitalWrite( WIRING_Z_DIRECTION_PIN, ( ( Steppers::z_direction == Steppers::Direction::TOWARDS_MAX ) ? HIGH : LOW ) );
}

uint32_t Steppers::CalculateRateDelayMicroseconds( float mmPerSec )
{
	// Calculate line length
	float distance = sqrt( ( Commands::delta_units.x * Commands::delta_units.x ) + ( Commands::delta_units.y * Commands::delta_units.y ) + ( Commands::delta_units.z * Commands::delta_units.z ) );
	uint32_t masterAxisSteps = 0;

	// Determine the dominant axis
	if ( Commands::delta_steps.x > Commands::delta_steps.y )
	{
		masterAxisSteps = ( Commands::delta_steps.z > Commands::delta_steps.x )
			? Commands::delta_steps.z
			: Commands::delta_steps.x;
	}
	else
	{
		masterAxisSteps = ( Commands::delta_steps.z > Commands::delta_steps.y )
			? Commands::delta_steps.z
			: Commands::delta_steps.y;
	}

	/*
	 * Calculate the delay (us) for which to pause between taking each step.
	 * (distance * 1000000.0) / mmPerSec = totalMovementTime (us)
	 * totalMovementTime / masterAxisSteps = delay time between steps for the master axis (us)
	 */
	return ( ( distance * 1000000.0 ) / ( mmPerSec * masterAxisSteps ) );
}

void Steppers::MoveToZero( void )
{
	Serial.println( "Moving to zero position" );

	Steppers::MoveToLimit( WIRING_X_LIMIT_MAX_PIN, WIRING_X_STEP_PIN, WIRING_X_DIRECTION_PIN, Steppers::Direction::TOWARDS_MAX );
	Steppers::MoveToLimit( WIRING_Y_LIMIT_MAX_PIN, WIRING_Y_STEP_PIN, WIRING_Y_DIRECTION_PIN, Steppers::Direction::TOWARDS_MAX );
	Steppers::MoveToLimit( WIRING_X_LIMIT_MIN_PIN, WIRING_X_STEP_PIN, WIRING_X_DIRECTION_PIN, Steppers::Direction::TOWARDS_MIN );
	Steppers::MoveToLimit( WIRING_Y_LIMIT_MIN_PIN, WIRING_Y_STEP_PIN, WIRING_Y_DIRECTION_PIN, Steppers::Direction::TOWARDS_MIN );

	Serial.println( "Moved to zero position" );
}

void Steppers::MoveToLimit( uint8_t limitPin, uint8_t stepperPin, uint8_t stepperDirectionPin, Steppers::Direction direction )
{
	Serial.print( "Moving towards " );
	Serial.println( ( ( direction == Steppers::Direction::TOWARDS_MAX ) ? "maximum limit" : "minimum limit" ) );
	while ( !ReadLimitSwitch( limitPin ) )
	{
		DoStep( stepperPin, stepperDirectionPin, direction );
	}

	// Slowly reverse until limit switch is released
	Serial.println( "Reversing" );
	while ( ReadLimitSwitch( limitPin ) )
	{
		DoStep( stepperPin, stepperDirectionPin, ( ( direction == Steppers::Direction::TOWARDS_MAX ) ? Steppers::Direction::TOWARDS_MIN : Steppers::Direction::TOWARDS_MAX ) );
	}
}

void Steppers::dda_move( long micro_delay )
{
	static int milli_delay;
	static long max_delta;

	Steppers::Enable();

	// figure out our deltas
	max_delta = max( Commands::delta_steps.x, Commands::delta_steps.y );
	max_delta = max( Commands::delta_steps.z, max_delta );

	// init stuff.
	long x_counter = -max_delta / 2;
	long y_counter = -max_delta / 2;
	long z_counter = -max_delta / 2;

	// our step flags
	bool x_can_step = 0;
	bool y_can_step = 0;
	bool z_can_step = 0;

	if ( micro_delay >= 16383 )
		milli_delay = micro_delay / 1000;
	else
		milli_delay = 0;

	// do our DDA line!
	do
	{
		x_can_step = !Steppers::IsTargetReached( Commands::current_steps.x, Commands::target_steps.x, Steppers::x_direction, WIRING_X_LIMIT_MIN_PIN, WIRING_X_LIMIT_MAX_PIN );
		y_can_step = !Steppers::IsTargetReached( Commands::current_steps.y, Commands::target_steps.y, Steppers::y_direction, WIRING_Y_LIMIT_MIN_PIN, WIRING_Y_LIMIT_MAX_PIN );
		z_can_step = !Steppers::IsTargetReached( Commands::current_steps.z, Commands::target_steps.z, Steppers::z_direction, WIRING_Z_LIMIT_MIN_PIN, WIRING_Z_LIMIT_MAX_PIN );

		if ( x_can_step )
		{
			x_counter += Commands::delta_steps.x;

			if ( x_counter > 0 )
			{
				DoStep( WIRING_X_STEP_PIN, WIRING_X_DIRECTION_PIN, Steppers::x_direction );
				x_counter -= max_delta;

				if ( Steppers::x_direction == Steppers::Direction::TOWARDS_MAX )
					Commands::current_steps.x++;
				else
					Commands::current_steps.x--;
			}
		}

		if ( y_can_step )
		{
			y_counter += Commands::delta_steps.y;

			if ( y_counter > 0 )
			{
				DoStep( WIRING_Y_STEP_PIN, WIRING_Y_DIRECTION_PIN, Steppers::y_direction );
				y_counter -= max_delta;

				if ( Steppers::y_direction == Steppers::Direction::TOWARDS_MAX )
					Commands::current_steps.y++;
				else
					Commands::current_steps.y--;
			}
		}

		if ( z_can_step )
		{
			z_counter += Commands::delta_steps.z;

			if ( z_counter > 0 )
			{
				if ( WIRING_Z_AXIS_SUPPORTED == 0 )
				{
					DoStep( WIRING_Z_STEP_PIN, WIRING_Z_DIRECTION_PIN, Steppers::z_direction );
				}
				z_counter -= max_delta;

				if ( Steppers::z_direction == Steppers::Direction::TOWARDS_MAX )
					Commands::current_steps.z++;
				else
					Commands::current_steps.z--;
			}
		}

		// wait for next step.
		if ( milli_delay > 0 )
			delay( milli_delay );
		else
			delayMicroseconds( micro_delay );
	} while ( x_can_step || y_can_step || z_can_step );

	// set our points to be the same
	Commands::current_units.x = Commands::target_units.x;
	Commands::current_units.y = Commands::target_units.y;
	Commands::current_units.z = Commands::target_units.z;
	Steppers::CalculateDeltas();
}

bool Steppers::IsTargetReached( uint32_t currentSteps, uint32_t targetSteps, Steppers::Direction direction, uint8_t limitMinPin, uint8_t limitMaxPin )
{
	// Serial.println( "============" );
	// Serial.print( "Current steps: " );
	// Serial.println( currentSteps );
	// Serial.print( "Target steps: " );
	// Serial.println( targetSteps );
	// Serial.print( "Direction: " );
	// Serial.println( ( direction == Steppers::Direction::TOWARDS_MAX ) ? "TOWARDS_MAX" : "TOWARDS_MIN" );
	// Serial.println( "============" );

	if ( ( direction == Steppers::Direction::TOWARDS_MIN ) && ( targetSteps >= currentSteps ) )
	{
		Serial.println( "Target reached" );
		return true;
	}
	else if ( ( direction == Steppers::Direction::TOWARDS_MAX ) && ( targetSteps <= currentSteps ) )
	{
		Serial.println( "Target reached" );
		return true;
	}

	if ( ReadLimitSwitch( limitMinPin ) || ReadLimitSwitch( limitMaxPin ) )
	{
		Serial.println( "Limit switch hit" );
		return true;
	}

	return false;
}

static uint32_t UnitsToSteps( float stepsPerUnit, float units )
{
	return stepsPerUnit * units;
}

static bool ReadLimitSwitch( uint8_t pin )
{
	// Dual read for crude debounce

	if ( PARAMETERS_LIMIT_SWITCHES_ACTIVE_LOW )
	{
		return !digitalRead( pin ) && !digitalRead( pin );
	}
	else
	{
		return digitalRead( pin ) && digitalRead( pin );
	}
}

static void DoStep( uint8_t stepperPin, uint8_t stepperDirectionPin, Steppers::Direction direction )
{
	switch ( ( ( ( direction == Steppers::Direction::TOWARDS_MAX ) ? 1 : 0 ) << 2 ) | ( digitalRead( stepperPin ) << 1 ) | ( digitalRead( stepperDirectionPin ) ) )
	{
	// 0 00 -> 10
	case 0b000:
	// 1 01 -> 11
	case 0b101:
		digitalWrite( stepperPin, HIGH );
		break;

	// 0 01 -> 00
	case 0b001:
	// 1 11 -> 10
	case 0b111:
		digitalWrite( stepperDirectionPin, LOW );
		break;

	// 0 10 -> 11
	case 0b010:
	// 1 00 -> 01
	case 0b100:
		digitalWrite( stepperDirectionPin, HIGH );
		break;

	// 0 11 -> 01
	case 0b011:
	// 1 10 -> 00
	case 0b110:
		digitalWrite( stepperPin, LOW );
		break;
	}
}
