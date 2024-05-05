#include <Arduino.h>

#include "common.h"

#include "Commands.hpp"
#include "Steppers.hpp"

Steppers::Point Steppers::currentPointMicrometres = { 0 };
Steppers::Point Steppers::targetPointMicrometres = { 0 };
Steppers::Point Steppers::deltaMicrometres = { 0 };

Steppers::Point Steppers::currentPointSteps = { 0 };
Steppers::Point Steppers::targetPointSteps = { 0 };
Steppers::Point Steppers::deltaSteps = { 0 };

Steppers::Direction Steppers::xDirection = Steppers::Direction::TOWARDS_MAX;
Steppers::Direction Steppers::yDirection = Steppers::Direction::TOWARDS_MAX;
Steppers::Direction Steppers::zDirection = Steppers::Direction::TOWARDS_MAX;

/**
 * @brief Convert a unit measurement into step counts.
 *
 * @param stepsPerUnit The number of steps per unit value.
 * @param units The unit measurement.
 * @return long The number of steps.
 */
static uint32_t UnitsToSteps( float stepsPerUnit, uint32_t units );

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
	Steppers::Disable();

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

	Steppers::Enable();
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
	if ( Steppers::deltaSteps.z > 0 )
	{
		return Steppers::CalculateRateDelayMicroseconds( PARAMETERS_MAX_Z_MM_PER_MIN );
	}
	else
	{
		return Steppers::CalculateRateDelayMicroseconds( PARAMETERS_MAX_XY_MM_PER_MIN );
	}
}

void Steppers::SetCurrentPoint( uint32_t xMicrometres, uint32_t yMicrometres, uint32_t zMicrometres )
{
	Steppers::currentPointMicrometres.x = xMicrometres;
	Steppers::currentPointMicrometres.y = yMicrometres;
	Steppers::currentPointMicrometres.z = zMicrometres;

	Steppers::CalculateDeltas();
}

void Steppers::SetTargetPoint( uint32_t xMicrometres, uint32_t yMicrometres, uint32_t zMicrometres )
{
	Steppers::targetPointMicrometres.x = xMicrometres;
	Steppers::targetPointMicrometres.y = yMicrometres;
	Steppers::targetPointMicrometres.z = zMicrometres;

	Steppers::CalculateDeltas();
}

void Steppers::CalculateDeltas( void )
{
	Serial.println( "Calculating deltas" );

	// Calculate the delta units between the current and target point
	Steppers::deltaMicrometres.x = abs( Steppers::targetPointMicrometres.x - Steppers::currentPointMicrometres.x );
	Steppers::deltaMicrometres.y = abs( Steppers::targetPointMicrometres.y - Steppers::currentPointMicrometres.y );
	Steppers::deltaMicrometres.z = abs( Steppers::targetPointMicrometres.z - Steppers::currentPointMicrometres.z );

	// Convert current units to step counts
	Steppers::currentPointSteps.x = UnitsToSteps( PARAMETERS_X_STEPS_PER_MICROMETRE, Steppers::currentPointMicrometres.x );
	Steppers::currentPointSteps.y = UnitsToSteps( PARAMETERS_Y_STEPS_PER_MICROMETRE, Steppers::currentPointMicrometres.y );
	Steppers::currentPointSteps.z = UnitsToSteps( PARAMETERS_Z_STEPS_PER_MICROMETRE, Steppers::currentPointMicrometres.z );

	// Convert target units to step counts
	Steppers::targetPointSteps.x = UnitsToSteps( PARAMETERS_X_STEPS_PER_MICROMETRE, Steppers::targetPointMicrometres.x );
	Steppers::targetPointSteps.y = UnitsToSteps( PARAMETERS_Y_STEPS_PER_MICROMETRE, Steppers::targetPointMicrometres.y );
	Steppers::targetPointSteps.z = UnitsToSteps( PARAMETERS_Z_STEPS_PER_MICROMETRE, Steppers::targetPointMicrometres.z );

	// Calculate the delta steps between the current and target point
	Steppers::deltaSteps.x = abs( Steppers::targetPointSteps.x - Steppers::currentPointSteps.x );
	Steppers::deltaSteps.y = abs( Steppers::targetPointSteps.y - Steppers::currentPointSteps.y );
	Steppers::deltaSteps.z = abs( Steppers::targetPointSteps.z - Steppers::currentPointSteps.z );

	// Determine the required direction of travel
	Steppers::xDirection = ( Steppers::targetPointMicrometres.x >= Steppers::currentPointMicrometres.x )
		? Steppers::Direction::TOWARDS_MAX
		: Steppers::Direction::TOWARDS_MIN;
	Steppers::yDirection = ( Steppers::targetPointMicrometres.y >= Steppers::currentPointMicrometres.y )
		? Steppers::Direction::TOWARDS_MAX
		: Steppers::Direction::TOWARDS_MIN;
	Steppers::zDirection = ( Steppers::targetPointMicrometres.z >= Steppers::currentPointMicrometres.z )
		? Steppers::Direction::TOWARDS_MAX
		: Steppers::Direction::TOWARDS_MIN;

	// Set direction pins
	digitalWrite( WIRING_X_DIRECTION_PIN, ( ( Steppers::xDirection == Steppers::Direction::TOWARDS_MAX ) ? HIGH : LOW ) );
	digitalWrite( WIRING_Y_DIRECTION_PIN, ( ( Steppers::yDirection == Steppers::Direction::TOWARDS_MAX ) ? HIGH : LOW ) );
	digitalWrite( WIRING_Z_DIRECTION_PIN, ( ( Steppers::zDirection == Steppers::Direction::TOWARDS_MAX ) ? HIGH : LOW ) );
}

uint32_t Steppers::CalculateRateDelayMicroseconds( uint32_t mmPerMinute )
{
	// Calculate line length
	uint32_t distance = sqrt( ( Steppers::deltaMicrometres.x * Steppers::deltaMicrometres.x ) + ( Steppers::deltaMicrometres.y * Steppers::deltaMicrometres.y ) + ( Steppers::deltaMicrometres.z * Steppers::deltaMicrometres.z ) );
	uint32_t masterAxisSteps = 0;

	// Determine the dominant axis
	if ( Steppers::deltaSteps.x > Steppers::deltaSteps.y )
	{
		masterAxisSteps = ( Steppers::deltaSteps.z > Steppers::deltaSteps.x )
			? Steppers::deltaSteps.z
			: Steppers::deltaSteps.x;
	}
	else
	{
		masterAxisSteps = ( Steppers::deltaSteps.z > Steppers::deltaSteps.y )
			? Steppers::deltaSteps.z
			: Steppers::deltaSteps.y;
	}

	/*
	 * Calculate the delay (us) for which to pause between taking each step.
	 * (distance * 60000000) / mmPerSec = totalMovementTime (us)
	 * totalMovementTime / masterAxisSteps = delay time between steps for the master axis (us)
	 */
	return ( ( distance * 60000000 ) / ( mmPerMinute * masterAxisSteps ) );
}

void Steppers::MoveToZero( void )
{
	Serial.println( "Moving to zero point" );

	Steppers::MoveToLimit( WIRING_X_LIMIT_MAX_PIN, WIRING_X_STEP_PIN, WIRING_X_DIRECTION_PIN, Steppers::Direction::TOWARDS_MAX );
	Steppers::MoveToLimit( WIRING_Y_LIMIT_MAX_PIN, WIRING_Y_STEP_PIN, WIRING_Y_DIRECTION_PIN, Steppers::Direction::TOWARDS_MAX );

	Steppers::MoveToLimit( WIRING_X_LIMIT_MIN_PIN, WIRING_X_STEP_PIN, WIRING_X_DIRECTION_PIN, Steppers::Direction::TOWARDS_MIN );
	Steppers::MoveToLimit( WIRING_Y_LIMIT_MIN_PIN, WIRING_Y_STEP_PIN, WIRING_Y_DIRECTION_PIN, Steppers::Direction::TOWARDS_MIN );

	Serial.println( "Moved to zero point" );
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
	max_delta = max( Steppers::deltaSteps.x, Steppers::deltaSteps.y );
	max_delta = max( Steppers::deltaSteps.z, max_delta );

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
		x_can_step = !Steppers::IsTargetReached( Steppers::currentPointSteps.x, Steppers::targetPointSteps.x, Steppers::xDirection, WIRING_X_LIMIT_MIN_PIN, WIRING_X_LIMIT_MAX_PIN );
		y_can_step = !Steppers::IsTargetReached( Steppers::currentPointSteps.y, Steppers::targetPointSteps.y, Steppers::yDirection, WIRING_Y_LIMIT_MIN_PIN, WIRING_Y_LIMIT_MAX_PIN );
		z_can_step = !Steppers::IsTargetReached( Steppers::currentPointSteps.z, Steppers::targetPointSteps.z, Steppers::zDirection, WIRING_Z_LIMIT_MIN_PIN, WIRING_Z_LIMIT_MAX_PIN );

		if ( x_can_step )
		{
			x_counter += Steppers::deltaSteps.x;

			if ( x_counter > 0 )
			{
				DoStep( WIRING_X_STEP_PIN, WIRING_X_DIRECTION_PIN, Steppers::xDirection );
				x_counter -= max_delta;

				if ( Steppers::xDirection == Steppers::Direction::TOWARDS_MAX )
					Steppers::currentPointSteps.x++;
				else
					Steppers::currentPointSteps.x--;
			}
		}

		if ( y_can_step )
		{
			y_counter += Steppers::deltaSteps.y;

			if ( y_counter > 0 )
			{
				DoStep( WIRING_Y_STEP_PIN, WIRING_Y_DIRECTION_PIN, Steppers::yDirection );
				y_counter -= max_delta;

				if ( Steppers::yDirection == Steppers::Direction::TOWARDS_MAX )
					Steppers::currentPointSteps.y++;
				else
					Steppers::currentPointSteps.y--;
			}
		}

		if ( z_can_step )
		{
			z_counter += Steppers::deltaSteps.z;

			if ( z_counter > 0 )
			{
				if ( WIRING_Z_AXIS_SUPPORTED == 0 )
				{
					DoStep( WIRING_Z_STEP_PIN, WIRING_Z_DIRECTION_PIN, Steppers::zDirection );
				}
				z_counter -= max_delta;

				if ( Steppers::zDirection == Steppers::Direction::TOWARDS_MAX )
					Steppers::currentPointSteps.z++;
				else
					Steppers::currentPointSteps.z--;
			}
		}

		// wait for next step.
		if ( milli_delay > 0 )
			delay( milli_delay );
		else
			delayMicroseconds( micro_delay );
	} while ( x_can_step || y_can_step || z_can_step );

	// set our points to be the same
	Steppers::currentPointMicrometres.x = Steppers::targetPointMicrometres.x;
	Steppers::currentPointMicrometres.y = Steppers::targetPointMicrometres.y;
	Steppers::currentPointMicrometres.z = Steppers::targetPointMicrometres.z;
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

static uint32_t UnitsToSteps( float stepsPerUnit, uint32_t units )
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
