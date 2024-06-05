#include <Arduino.h>

#include "common.h"

#include "Commands.hpp"
#include "Steppers.hpp"

void setup()
{
	Serial.begin( 115200 );

	Commands::ClearCommandBuffer();

	Serial.println( "Initialising steppers..." );
	Steppers::Initialise();

	Serial.print( "Executing " );
	Serial.println( Commands::COMMAND_ABSOLUTE_POSITION_MODE );
	Commands::Execute( Commands::COMMAND_ABSOLUTE_POSITION_MODE, strlen( Commands::COMMAND_ABSOLUTE_POSITION_MODE ) );

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
