#include <Arduino.h>

#include "common.h"

#include "Commands.hpp"
#include "Steppers.hpp"

void DrawTestPattern()
{
	static uint8_t testRateDelay = STEPPERS_MIN_MICROSECOND_DELAY;

	Steppers::SetTargetPoint( 325000, 305000, 0 );
	Steppers::MoveToTarget( testRateDelay );

	delay( 1000 );

	Steppers::SetTargetPoint( 100000, 100000, 0 );
	Steppers::MoveToTarget( testRateDelay );
	Steppers::SetTargetPoint( 125000, 100000, 0 );
	Steppers::MoveToTarget( testRateDelay );
	Steppers::SetTargetPoint( 150000, 125000, 0 );
	Steppers::MoveToTarget( testRateDelay );
	Steppers::SetTargetPoint( 150000, 150000, 0 );
	Steppers::MoveToTarget( testRateDelay );
	Steppers::SetTargetPoint( 125000, 175000, 0 );
	Steppers::MoveToTarget( testRateDelay );
	Steppers::SetTargetPoint( 100000, 175000, 0 );
	Steppers::MoveToTarget( testRateDelay );
	Steppers::SetTargetPoint( 75000, 150000, 0 );
	Steppers::MoveToTarget( testRateDelay );
	Steppers::SetTargetPoint( 75000, 125000, 0 );
	Steppers::MoveToTarget( testRateDelay );
	// ---
	Steppers::SetTargetPoint( 100000, 100000, 0 );
	Steppers::MoveToTarget( testRateDelay );
	Steppers::SetTargetPoint( 125000, 100000, 0 );
	Steppers::MoveToTarget( testRateDelay );
	Steppers::SetTargetPoint( 150000, 125000, 0 );
	Steppers::MoveToTarget( testRateDelay );
	Steppers::SetTargetPoint( 150000, 150000, 0 );
	Steppers::MoveToTarget( testRateDelay );
	Steppers::SetTargetPoint( 125000, 175000, 0 );
	Steppers::MoveToTarget( testRateDelay );
	Steppers::SetTargetPoint( 100000, 175000, 0 );
	Steppers::MoveToTarget( testRateDelay );
	Steppers::SetTargetPoint( 75000, 150000, 0 );
	Steppers::MoveToTarget( testRateDelay );
	Steppers::SetTargetPoint( 75000, 125000, 0 );
	Steppers::MoveToTarget( testRateDelay );
	// ---
	Steppers::SetTargetPoint( 100000, 100000, 0 );
	Steppers::MoveToTarget( testRateDelay );
	Steppers::SetTargetPoint( 125000, 100000, 0 );
	Steppers::MoveToTarget( testRateDelay );
	Steppers::SetTargetPoint( 150000, 125000, 0 );
	Steppers::MoveToTarget( testRateDelay );
	Steppers::SetTargetPoint( 150000, 150000, 0 );
	Steppers::MoveToTarget( testRateDelay );
	Steppers::SetTargetPoint( 125000, 175000, 0 );
	Steppers::MoveToTarget( testRateDelay );
	Steppers::SetTargetPoint( 100000, 175000, 0 );
	Steppers::MoveToTarget( testRateDelay );
	Steppers::SetTargetPoint( 75000, 150000, 0 );
	Steppers::MoveToTarget( testRateDelay );
	Steppers::SetTargetPoint( 75000, 125000, 0 );
	Steppers::MoveToTarget( testRateDelay );
	// ---
	Steppers::SetTargetPoint( 100000, 100000, 0 );
	Steppers::MoveToTarget( testRateDelay );
	Steppers::SetTargetPoint( 125000, 100000, 0 );
	Steppers::MoveToTarget( testRateDelay );
	Steppers::SetTargetPoint( 150000, 125000, 0 );
	Steppers::MoveToTarget( testRateDelay );
	Steppers::SetTargetPoint( 150000, 150000, 0 );
	Steppers::MoveToTarget( testRateDelay );
	Steppers::SetTargetPoint( 125000, 175000, 0 );
	Steppers::MoveToTarget( testRateDelay );
	Steppers::SetTargetPoint( 100000, 175000, 0 );
	Steppers::MoveToTarget( testRateDelay );
	Steppers::SetTargetPoint( 75000, 150000, 0 );
	Steppers::MoveToTarget( testRateDelay );
	Steppers::SetTargetPoint( 75000, 125000, 0 );
	Steppers::MoveToTarget( testRateDelay );
	// ---
	Steppers::SetTargetPoint( 100000, 100000, 0 );
	Steppers::MoveToTarget( testRateDelay );
	Steppers::SetTargetPoint( 125000, 100000, 0 );
	Steppers::MoveToTarget( testRateDelay );
	Steppers::SetTargetPoint( 150000, 125000, 0 );
	Steppers::MoveToTarget( testRateDelay );
	Steppers::SetTargetPoint( 150000, 150000, 0 );
	Steppers::MoveToTarget( testRateDelay );
	Steppers::SetTargetPoint( 125000, 175000, 0 );
	Steppers::MoveToTarget( testRateDelay );
	Steppers::SetTargetPoint( 100000, 175000, 0 );
	Steppers::MoveToTarget( testRateDelay );
	Steppers::SetTargetPoint( 75000, 150000, 0 );
	Steppers::MoveToTarget( testRateDelay );
	Steppers::SetTargetPoint( 75000, 125000, 0 );
	Steppers::MoveToTarget( testRateDelay );
	// ---
	Steppers::SetTargetPoint( 100000, 100000, 0 );
	Steppers::MoveToTarget( testRateDelay );
	Steppers::SetTargetPoint( 125000, 100000, 0 );
	Steppers::MoveToTarget( testRateDelay );
	Steppers::SetTargetPoint( 150000, 125000, 0 );
	Steppers::MoveToTarget( testRateDelay );
	Steppers::SetTargetPoint( 150000, 150000, 0 );
	Steppers::MoveToTarget( testRateDelay );
	Steppers::SetTargetPoint( 125000, 175000, 0 );
	Steppers::MoveToTarget( testRateDelay );
	Steppers::SetTargetPoint( 100000, 175000, 0 );
	Steppers::MoveToTarget( testRateDelay );
	Steppers::SetTargetPoint( 75000, 150000, 0 );
	Steppers::MoveToTarget( testRateDelay );
	Steppers::SetTargetPoint( 75000, 125000, 0 );
	Steppers::MoveToTarget( testRateDelay );
	// ---
	Steppers::SetTargetPoint( 100000, 100000, 0 );
	Steppers::MoveToTarget( testRateDelay );
	Steppers::SetTargetPoint( 125000, 100000, 0 );
	Steppers::MoveToTarget( testRateDelay );
	Steppers::SetTargetPoint( 150000, 125000, 0 );
	Steppers::MoveToTarget( testRateDelay );
	Steppers::SetTargetPoint( 150000, 150000, 0 );
	Steppers::MoveToTarget( testRateDelay );
	Steppers::SetTargetPoint( 125000, 175000, 0 );
	Steppers::MoveToTarget( testRateDelay );
	Steppers::SetTargetPoint( 100000, 175000, 0 );
	Steppers::MoveToTarget( testRateDelay );
	Steppers::SetTargetPoint( 75000, 150000, 0 );
	Steppers::MoveToTarget( testRateDelay );
	Steppers::SetTargetPoint( 75000, 125000, 0 );
	Steppers::MoveToTarget( testRateDelay );
	// ---
	Steppers::SetTargetPoint( 100000, 100000, 0 );
	Steppers::MoveToTarget( testRateDelay );
	Steppers::SetTargetPoint( 125000, 100000, 0 );
	Steppers::MoveToTarget( testRateDelay );
	Steppers::SetTargetPoint( 150000, 125000, 0 );
	Steppers::MoveToTarget( testRateDelay );
	Steppers::SetTargetPoint( 150000, 150000, 0 );
	Steppers::MoveToTarget( testRateDelay );
	Steppers::SetTargetPoint( 125000, 175000, 0 );
	Steppers::MoveToTarget( testRateDelay );
	Steppers::SetTargetPoint( 100000, 175000, 0 );
	Steppers::MoveToTarget( testRateDelay );
	Steppers::SetTargetPoint( 75000, 150000, 0 );
	Steppers::MoveToTarget( testRateDelay );
	Steppers::SetTargetPoint( 75000, 125000, 0 );
	Steppers::MoveToTarget( testRateDelay );
	// ---
	Steppers::SetTargetPoint( 100000, 100000, 0 );
	Steppers::MoveToTarget( testRateDelay );
	Steppers::SetTargetPoint( 125000, 100000, 0 );
	Steppers::MoveToTarget( testRateDelay );
	Steppers::SetTargetPoint( 150000, 125000, 0 );
	Steppers::MoveToTarget( testRateDelay );
	Steppers::SetTargetPoint( 150000, 150000, 0 );
	Steppers::MoveToTarget( testRateDelay );
	Steppers::SetTargetPoint( 125000, 175000, 0 );
	Steppers::MoveToTarget( testRateDelay );
	Steppers::SetTargetPoint( 100000, 175000, 0 );
	Steppers::MoveToTarget( testRateDelay );
	Steppers::SetTargetPoint( 75000, 150000, 0 );
	Steppers::MoveToTarget( testRateDelay );
	Steppers::SetTargetPoint( 75000, 125000, 0 );
	Steppers::MoveToTarget( testRateDelay );

	delay( 1000 );

	Commands::Execute( "G28", strlen( "G28" ) );

	delay( 1000 );

	Steppers::SetTargetPoint( 325000, 305000, 0 );
	Steppers::MoveToTarget( STEPPERS_MIN_MICROSECOND_DELAY );
}

void setup()
{
	Serial.begin( 115200 );

	Commands::ClearCommandBuffer();

	Serial.println( "Initialising steppers..." );
	Steppers::Initialise();

	Serial.print( "Executing " );
	Serial.println( Commands::COMMAND_ABSOLUTE_POSITION_MODE );
	Commands::Execute( Commands::COMMAND_ABSOLUTE_POSITION_MODE, strlen( Commands::COMMAND_ABSOLUTE_POSITION_MODE ) );

	// DrawTestPattern();

	Serial.println( "Completed setup" );
}

void loop()
{
	static uint32_t idleIterations = 0;
	static bool receivedCompleteCommand = false;

	if ( Serial.available() > 0 )
	{
		idleIterations = 0;
		receivedCompleteCommand = Commands::ReceiveByte();
	}
	else
	{
		idleIterations++;
		delayMicroseconds( 100 );

		if ( receivedCompleteCommand )
		{
			Commands::ExecuteReceived();
			Commands::ClearCommandBuffer();
			receivedCompleteCommand = false;
		}

#if ( PARAMETERS_SLEEP_IF_IDLE == 1 )
		// Turn off steppers if no commands received for too long
		if ( idleIterations >= PARAMETERS_IDLE_ITERATIONS_BEFORE_SLEEP )
		{
			Steppers::Disable();
		}
#endif
	}
}
