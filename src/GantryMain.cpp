#include <Arduino.h>

#include "common.h"

#include "Commands.hpp"
#include "stepper_control.hpp"

void setup()
{
	Serial.begin( 115200 );

	Commands::ClearCommandBuffer();
	init_steppers();
	Commands::Execute( Commands::COMMAND_ABSOLUTE_POSITION, strlen( Commands::COMMAND_ABSOLUTE_POSITION ) );
	Serial.println( "start" );
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
		}

#if ( PARAMETERS_SLEEP_IF_IDLE == 1 )
		// Turn off steppers if no commands received for too long
		if ( idleIterations >= PARAMETERS_IDLE_ITERATIONS_BEFORE_SLEEP )
		{
			disable_steppers();
		}
#endif
	}
}
