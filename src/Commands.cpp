#include <Arduino.h>

#include "common.h"

#include "Commands.hpp"
#include "Steppers.hpp"

const char Commands::COMMAND_ABSOLUTE_POSITION_MODE[] = "G90";

static char commandString[ COMMAND_MAX_LENGTH ] = { '\0' };
static uint8_t receivedBytes = 0;

/**
 * @brief Check whether a given token is contained within a provided string.
 *
 * @param token The token to search for.
 * @param string The string to search within.
 * @param stringLength The length of the string.
 * @return true if the string contains the token. \
 * @return false otherwise.
 */
static bool StringContains( char token, const char *string, const size_t stringLength );
/**
 * @brief Extract the numeric payload following a specified token.
 *
 * @param token The proceeding token.
 * @param string The string to search within.
 * @param stringLength The length of the string.
 * @return double The extracted payload.
 */
static double ExtractNumericPayload( char token, const char *string, const size_t stringLength );

void Commands::ClearCommandBuffer( void )
{
	memset( commandString, '\0', COMMAND_MAX_LENGTH );
	receivedBytes = 0;
}

bool Commands::ReceiveByte( void )
{
	static char receivedByte;
	receivedByte = Serial.read();

	if ( receivedByte != COMMAND_TERMINATOR )
	{
		// TODO: wtf does this CANCEL mean?
		if ( receivedByte == 0x18 )
		{
			Serial.println( "Grbl 0" );
		}
		else
		{
			commandString[ receivedBytes ] = receivedByte;
			receivedBytes++;
		}
	}

	return ( receivedByte == COMMAND_TERMINATOR );
}

void Commands::ExecuteReceived( void )
{
	Commands::Execute( commandString, receivedBytes );
}

// Read the string and execute instructions
void Commands::Execute( const char *command, const size_t commandLength )
{
	/**
	 * @brief Whether the system is in incremental (`false`) or absolute (`true`) mode.
	 */
	static bool readValuesAsAbsolute = false;

	static uint32_t feedRateUnitsPerSecond = 0;
	static uint32_t feedRateDelayMicroseconds = 0;

	if ( command == NULL )
	{
		Serial.println( "ERROR: Trying to execute NULL command!" );
		return;
	}

	// the character / means delete block... used for comments and stuff.
	if ( command[ 0 ] == '/' )
	{
		Serial.println( "ok" );
		return;
	}

	Steppers::Point targetPointMicrometres;
	targetPointMicrometres.x = 0;
	targetPointMicrometres.y = 0;
	targetPointMicrometres.z = 0;

	uint8_t code = 0;

	// Ensure the received command is valid G-Code
	if ( !StringContains( '$', command, commandLength ) && ( StringContains( 'G', command, commandLength ) || StringContains( 'X', command, commandLength ) || StringContains( 'Y', command, commandLength ) || StringContains( 'Z', command, commandLength ) ) )
	{
		code = ( int ) ExtractNumericPayload( 'G', command, commandLength );

		/*
		 * Get co-ordinates if required by the code type given
		 */
		switch ( code )
		{
			// Positioning at rapid travel
			case 0:
			// Linear interpolation using a feed rate
			case 1:
			// Return to reference point through an intermediate point
			case 30:
				Steppers::Point currentPointMicrometres = Steppers::GetCurrentPoint();

				if ( readValuesAsAbsolute )
				{
					targetPointMicrometres.x = ( StringContains( 'X', command, commandLength ) )
						? ExtractNumericPayload( 'X', command, commandLength )
						: currentPointMicrometres.x;

					targetPointMicrometres.x = ( StringContains( 'Y', command, commandLength ) )
						? ExtractNumericPayload( 'Y', command, commandLength )
						: currentPointMicrometres.y;

					targetPointMicrometres.z = ( StringContains( 'Z', command, commandLength ) )
						? ExtractNumericPayload( 'Z', command, commandLength )
						: currentPointMicrometres.z;
				}
				else
				{
					targetPointMicrometres.x = ExtractNumericPayload( 'X', command, commandLength ) + currentPointMicrometres.x;
					targetPointMicrometres.y = ExtractNumericPayload( 'Y', command, commandLength ) + currentPointMicrometres.y;
					targetPointMicrometres.z = ExtractNumericPayload( 'Z', command, commandLength ) + currentPointMicrometres.z;
				}
				break;
		}

		/*
		 * Perform translation
		 */
		switch ( code )
		{
			// Positioning at rapid travel
			// Note that this shares the same handler as for G01; appropriate feed rate logic is implemented
			case 0:
			// Linear interpolation using a feed rate
			case 1:
				Steppers::SetTargetPoint( targetPointMicrometres.x, targetPointMicrometres.y, targetPointMicrometres.z );

				// Check if a feed rate has been specified
				if ( StringContains( 'G', command, commandLength ) )
				{
					// Linear interpolation using a feed rate
					if ( code == 1 )
					{
						// * G-Code standard format is units per SECOND!
						feedRateUnitsPerSecond = ExtractNumericPayload( 'F', command, commandLength );

						feedRateDelayMicroseconds = ( feedRateUnitsPerSecond > 0 )
							? Steppers::CalculateRateDelayMicroseconds( feedRateUnitsPerSecond )
							: Steppers::GetMinRateDelayMicroseconds();
					}
					else
					{
						// Use maximum feed rate
						feedRateDelayMicroseconds = Steppers::GetMinRateDelayMicroseconds();
					}
				}
				else
				{
					feedRateDelayMicroseconds = ( feedRateUnitsPerSecond > 0 )
						? Steppers::CalculateRateDelayMicroseconds( feedRateUnitsPerSecond )
						: Steppers::GetMinRateDelayMicroseconds();
				}

				// Perform the movement at the appropriate rate
				Steppers::MoveToPoint( feedRateDelayMicroseconds );
				break;

			// Dwell
			case 4:
				delay( ( int ) ExtractNumericPayload( 'P', command, commandLength ) );
				break;

			// Return to reference point
			case 28:
				Steppers::SetTargetPoint( 0, 0, 0 );
				Steppers::MoveToZero();
				break;

			// Return to reference point through an intermediate point
			case 30:
				// Move to the intermediate point
				Steppers::SetTargetPoint( targetPointMicrometres.x, targetPointMicrometres.y, targetPointMicrometres.z );
				Steppers::MoveToPoint( Steppers::GetMinRateDelayMicroseconds() );

				// Return to reference point
				Steppers::SetTargetPoint( 0, 0, 0 );
				Steppers::MoveToZero();
				break;

			// Set distance mode absolute
			case 90:
				readValuesAsAbsolute = true;
				break;

			// Set distance mode incremental
			case 91:
				readValuesAsAbsolute = false;
				break;

			// Reposition origin point
			case 92:
				Steppers::SetCurrentPoint( 0, 0, 0 );
				break;

			default:
				Serial.print( "Unhandled G-Code: " );
				Serial.println( code, DEC );
				break;
		}
	}

	if ( ( code == 0 ) && ( commandLength == 1 ) )
	{
		Serial.println( "Start" );
	}
	else
	{
		Serial.println( "Done!" );
	}
}

static bool StringContains( char token, const char *string, const size_t stringLength )
{
	for ( uint8_t i = 0; i < stringLength; i++ )
	{
		if ( string[ i ] == token )
		{
			return true;
		}
	}

	return false;
}

static double ExtractNumericPayload( char token, const char *string, const size_t stringLength )
{
	char extracted[ COMMAND_NUMERIC_PAYLOAD_MAX_LENGTH ] = { '\0' };

	for ( uint8_t i = 0; i < stringLength; i++ )
	{
		if ( string[ i ] != token )
		{
			continue;
		}

		// The token has been found, so extract the payload
		i++;
		uint8_t k = 0;
		while ( ( i < stringLength ) && ( k < ( COMMAND_NUMERIC_PAYLOAD_MAX_LENGTH - 1 ) ) )
		{
			if ( ( string[ i ] == 0 ) || ( string[ i ] == ' ' ) )
			{
				break;
			}

			extracted[ k ] = string[ i ];
			i++;
			k++;
		}
		extracted[ k ] = 0;

		return strtod( extracted, NULL );
	}

	return 0;
}
