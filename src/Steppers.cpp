#include <Arduino.h>

#include "common.h"

#include "Commands.hpp"
#include "Steppers.hpp"

static Steppers::Point currentPointMicrometres = { 0 };
static Steppers::Point targetPointMicrometres = { 0 };
static Steppers::Point deltaMicrometres = { 0 };

static Steppers::Point currentPointSteps = { 0 };
static Steppers::Point targetPointSteps = { 0 };
static Steppers::Point deltaSteps = { 0 };

static Steppers::Direction xDirection = Steppers::Direction::TOWARDS_MAX;
static Steppers::Direction yDirection = Steppers::Direction::TOWARDS_MAX;
static Steppers::Direction zDirection = Steppers::Direction::TOWARDS_MAX;

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
	if ( deltaSteps.z > 0 )
	{
		return Steppers::CalculateRateDelayMicroseconds( PARAMETERS_MAX_Z_MM_PER_MIN );
	}
	else
	{
		return Steppers::CalculateRateDelayMicroseconds( PARAMETERS_MAX_XY_MM_PER_MIN );
	}
}

Steppers::Point Steppers::GetCurrentPoint( void )
{
	return currentPointMicrometres;
}

void Steppers::SetCurrentPoint( uint32_t xMicrometres, uint32_t yMicrometres, uint32_t zMicrometres )
{
	currentPointMicrometres.x = xMicrometres;
	currentPointMicrometres.y = yMicrometres;
	currentPointMicrometres.z = zMicrometres;

	Steppers::CalculateDeltas();
}

void Steppers::SetTargetPoint( uint32_t xMicrometres, uint32_t yMicrometres, uint32_t zMicrometres )
{
	targetPointMicrometres.x = xMicrometres;
	targetPointMicrometres.y = yMicrometres;
	targetPointMicrometres.z = zMicrometres;

	Steppers::CalculateDeltas();
}

void Steppers::CalculateDeltas( void )
{
	Serial.println( "Calculating deltas" );

	// Calculate the delta units between the current and target point
	deltaMicrometres.x = abs( ( int32_t ) ( targetPointMicrometres.x - currentPointMicrometres.x ) );
	deltaMicrometres.y = abs( ( int32_t ) ( targetPointMicrometres.y - currentPointMicrometres.y ) );
	deltaMicrometres.z = abs( ( int32_t ) ( targetPointMicrometres.z - currentPointMicrometres.z ) );

	Serial.print( "Delta: " );
	Serial.print( deltaMicrometres.x );
	Serial.print( "um (x) " );
	Serial.print( deltaMicrometres.y );
	Serial.print( "um (y) " );
	Serial.print( deltaMicrometres.z );
	Serial.println( "um (z)" );

	// Convert current units to step counts
	currentPointSteps.x = UnitsToSteps( PARAMETERS_X_STEPS_PER_MICROMETRE, currentPointMicrometres.x );
	currentPointSteps.y = UnitsToSteps( PARAMETERS_Y_STEPS_PER_MICROMETRE, currentPointMicrometres.y );
	currentPointSteps.z = UnitsToSteps( PARAMETERS_Z_STEPS_PER_MICROMETRE, currentPointMicrometres.z );

	// Convert target units to step counts
	targetPointSteps.x = UnitsToSteps( PARAMETERS_X_STEPS_PER_MICROMETRE, targetPointMicrometres.x );
	targetPointSteps.y = UnitsToSteps( PARAMETERS_Y_STEPS_PER_MICROMETRE, targetPointMicrometres.y );
	targetPointSteps.z = UnitsToSteps( PARAMETERS_Z_STEPS_PER_MICROMETRE, targetPointMicrometres.z );

	// Calculate the delta steps between the current and target point
	deltaSteps.x = abs( ( int32_t ) ( targetPointSteps.x - currentPointSteps.x ) );
	deltaSteps.y = abs( ( int32_t ) ( targetPointSteps.y - currentPointSteps.y ) );
	deltaSteps.z = abs( ( int32_t ) ( targetPointSteps.z - currentPointSteps.z ) );

	Serial.print( "Delta: " );
	Serial.print( deltaSteps.x );
	Serial.print( "steps (x) " );
	Serial.print( deltaSteps.y );
	Serial.print( "steps (y) " );
	Serial.print( deltaSteps.z );
	Serial.println( "steps (z)" );

	// Determine the required direction of travel
	xDirection = ( targetPointMicrometres.x >= currentPointMicrometres.x )
		? Steppers::Direction::TOWARDS_MAX
		: Steppers::Direction::TOWARDS_MIN;
	yDirection = ( targetPointMicrometres.y >= currentPointMicrometres.y )
		? Steppers::Direction::TOWARDS_MAX
		: Steppers::Direction::TOWARDS_MIN;
	zDirection = ( targetPointMicrometres.z >= currentPointMicrometres.z )
		? Steppers::Direction::TOWARDS_MAX
		: Steppers::Direction::TOWARDS_MIN;

	// Set direction pins
	digitalWrite( WIRING_X_DIRECTION_PIN, ( ( xDirection == Steppers::Direction::TOWARDS_MAX ) ? HIGH : LOW ) );
	digitalWrite( WIRING_Y_DIRECTION_PIN, ( ( yDirection == Steppers::Direction::TOWARDS_MAX ) ? HIGH : LOW ) );
	digitalWrite( WIRING_Z_DIRECTION_PIN, ( ( zDirection == Steppers::Direction::TOWARDS_MAX ) ? HIGH : LOW ) );
}

uint32_t Steppers::CalculateRateDelayMicroseconds( uint32_t mmPerMinute )
{
	return STEPPERS_MIN_MICROSECOND_DELAY;

	// Calculate line length
	uint32_t distance = sqrt( ( deltaMicrometres.x * deltaMicrometres.x ) + ( deltaMicrometres.y * deltaMicrometres.y ) + ( deltaMicrometres.z * deltaMicrometres.z ) );
	uint32_t masterAxisSteps = 0;

	// Determine the dominant axis
	if ( deltaSteps.x > deltaSteps.y )
	{
		masterAxisSteps = ( deltaSteps.z > deltaSteps.x )
			? deltaSteps.z
			: deltaSteps.x;
	}
	else
	{
		masterAxisSteps = ( deltaSteps.z > deltaSteps.y )
			? deltaSteps.z
			: deltaSteps.y;
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

	Steppers::MoveToLimit( WIRING_X_LIMIT_MIN_PIN, WIRING_X_STEP_PIN, WIRING_X_DIRECTION_PIN, Steppers::Direction::TOWARDS_MIN );
	Steppers::MoveToLimit( WIRING_Y_LIMIT_MIN_PIN, WIRING_Y_STEP_PIN, WIRING_Y_DIRECTION_PIN, Steppers::Direction::TOWARDS_MIN );

	Steppers::SetCurrentPoint( 0, 0, 0 );

	Serial.println( "Moved to zero point" );
}

void Steppers::MoveToLimit( uint8_t limitPin, uint8_t stepperPin, uint8_t stepperDirectionPin, Steppers::Direction direction )
{
	Serial.println();
	Serial.print( "Moving towards " );
	Serial.println( ( ( direction == Steppers::Direction::TOWARDS_MAX ) ? "maximum limit" : "minimum limit" ) );
	while ( !ReadLimitSwitch( limitPin ) )
	{
		DoStep( stepperPin, stepperDirectionPin, direction );
	}
	Serial.println( "Hit limit" );

	delay( 250 );

	Serial.print( "Limit switches: " );
	Serial.print( ReadLimitSwitch( WIRING_X_LIMIT_MIN_PIN ) );
	Serial.print( ReadLimitSwitch( WIRING_X_LIMIT_MAX_PIN ) );
	Serial.print( ReadLimitSwitch( WIRING_Y_LIMIT_MIN_PIN ) );
	Serial.println( ReadLimitSwitch( WIRING_Y_LIMIT_MAX_PIN ) );

	// Slowly reverse until limit switch is released
	Steppers::Direction reverseDirection = ( direction == Steppers::Direction::TOWARDS_MAX )
		? Steppers::Direction::TOWARDS_MIN
		: Steppers::Direction::TOWARDS_MAX;
	Serial.print( "Reversing towards " );
	Serial.println( ( ( reverseDirection == Steppers::Direction::TOWARDS_MAX ) ? "maximum limit" : "minimum limit" ) );
	while ( ReadLimitSwitch( limitPin ) )
	{
		DoStep( stepperPin, stepperDirectionPin, reverseDirection );
	}

	Serial.print( "Limit switches: " );
	Serial.print( ReadLimitSwitch( WIRING_X_LIMIT_MIN_PIN ) );
	Serial.print( ReadLimitSwitch( WIRING_X_LIMIT_MAX_PIN ) );
	Serial.print( ReadLimitSwitch( WIRING_Y_LIMIT_MIN_PIN ) );
	Serial.println( ReadLimitSwitch( WIRING_Y_LIMIT_MAX_PIN ) );

	Serial.println( "Done" );
}

void Steppers::MoveToTarget( uint32_t rateDelayMicroseconds )
{
	Serial.println();
	Serial.print( "Moving to target: " );
	Serial.print( targetPointMicrometres.x );
	Serial.print( "um (x) " );
	Serial.print( targetPointMicrometres.y );
	Serial.print( "um (y) " );
	Serial.print( targetPointMicrometres.z );
	Serial.println( "um (z)" );

	Serial.print( "Moving with rate delay: " );
	Serial.println( rateDelayMicroseconds );

	Steppers::Enable();

	// Determine the largest delta distance
	uint32_t maxDeltaSteps = max( max( deltaSteps.x, deltaSteps.y ), deltaSteps.z );

	int32_t xStepCounter = -( maxDeltaSteps / 2 );
	int32_t yStepCounter = -( maxDeltaSteps / 2 );
	int32_t zStepCounter = -( maxDeltaSteps / 2 );

	bool xCanStep = false;
	bool yCanStep = false;
	bool zCanStep = false;

	// Ensure that the required microsecond delay does not exceed precision limit
	uint32_t rateDelayMilliseconds = ( rateDelayMicroseconds >= STEPPERS_MAX_MICROSECOND_DELAY )
		? rateDelayMicroseconds / 1000
		: 0;

	// Execute the DDA algorithm
	do
	{
		xCanStep = !Steppers::IsTargetReached( currentPointSteps.x, targetPointSteps.x, xDirection, WIRING_X_LIMIT_MIN_PIN, WIRING_X_LIMIT_MAX_PIN );
		yCanStep = !Steppers::IsTargetReached( currentPointSteps.y, targetPointSteps.y, yDirection, WIRING_Y_LIMIT_MIN_PIN, WIRING_Y_LIMIT_MAX_PIN );
		zCanStep = !Steppers::IsTargetReached( currentPointSteps.z, targetPointSteps.z, zDirection, WIRING_Z_LIMIT_MIN_PIN, WIRING_Z_LIMIT_MAX_PIN );

		if ( xCanStep )
		{
			xStepCounter += deltaSteps.x;

			if ( xStepCounter > 0 )
			{
				DoStep( WIRING_X_STEP_PIN, WIRING_X_DIRECTION_PIN, xDirection );
				xStepCounter -= maxDeltaSteps;

				if ( xDirection == Steppers::Direction::TOWARDS_MAX )
				{
					currentPointSteps.x++;
				}
				else
				{
					currentPointSteps.x--;
				}
			}
		}

		if ( yCanStep )
		{
			yStepCounter += deltaSteps.y;

			if ( yStepCounter > 0 )
			{
				DoStep( WIRING_Y_STEP_PIN, WIRING_Y_DIRECTION_PIN, yDirection );
				yStepCounter -= maxDeltaSteps;

				if ( yDirection == Steppers::Direction::TOWARDS_MAX )
				{
					currentPointSteps.y++;
				}
				else
				{
					currentPointSteps.y--;
				}
			}
		}

		if ( zCanStep )
		{
			zStepCounter += deltaSteps.z;

			if ( zStepCounter > 0 )
			{
				if ( WIRING_Z_AXIS_SUPPORTED )
				{
					DoStep( WIRING_Z_STEP_PIN, WIRING_Z_DIRECTION_PIN, zDirection );
				}
				zStepCounter -= maxDeltaSteps;

				if ( zDirection == Steppers::Direction::TOWARDS_MAX )
				{
					currentPointSteps.z++;
				}
				else
				{
					currentPointSteps.z--;
				}
			}
		}

		// Delay before taking the next step
		if ( rateDelayMilliseconds > 0 )
		{
			delay( rateDelayMilliseconds );
		}
		else
		{
			delayMicroseconds( rateDelayMicroseconds );
		}
	} while ( xCanStep || yCanStep || zCanStep );

	// Update the current point
	Steppers::SetCurrentPoint( targetPointMicrometres.x, targetPointMicrometres.y, targetPointMicrometres.z );
	Steppers::CalculateDeltas();
}

bool Steppers::IsTargetReached( uint32_t currentSteps, uint32_t targetSteps, Steppers::Direction direction, uint8_t limitMinPin, uint8_t limitMaxPin )
{
	if ( ( direction == Steppers::Direction::TOWARDS_MIN ) && ( targetSteps >= currentSteps ) )
	{
		// Serial.println( "Target reached (min)" );
		return true;
	}
	else if ( ( direction == Steppers::Direction::TOWARDS_MAX ) && ( targetSteps <= currentSteps ) )
	{
		// Serial.println( "Target reached (max)" );
		return true;
	}

	if ( ( direction == Steppers::Direction::TOWARDS_MIN ) && ( ReadLimitSwitch( limitMinPin ) ) )
	{
		Serial.println( "Limit switch hit (min)" );
		return true;
	}
	else if ( ( direction == Steppers::Direction::TOWARDS_MAX ) && ( ReadLimitSwitch( limitMaxPin ) ) )
	{
		Serial.println( "Limit switch hit (max)" );
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
