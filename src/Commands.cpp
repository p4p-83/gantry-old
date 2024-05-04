#include <Arduino.h>

#include "common.h"

#include "Commands.hpp"
#include "stepper_control.hpp"

const char Commands::COMMAND_ABSOLUTE_POSITION[] = "G90";

Commands::FloatPoint Commands::current_units;
Commands::FloatPoint Commands::target_units;
Commands::FloatPoint Commands::delta_units;

Commands::FloatPoint Commands::current_steps;
Commands::FloatPoint Commands::target_steps;
Commands::FloatPoint Commands::delta_steps;

// our direction vars
uint8_t Commands::x_direction = 1;
uint8_t Commands::y_direction = 1;
uint8_t Commands::z_direction = 1;

float Commands::x_units = PARAMETERS_X_STEPS_PER_MM;
float Commands::y_units = PARAMETERS_Y_STEPS_PER_MM;
float Commands::z_units = PARAMETERS_Z_STEPS_PER_MM;

static boolean abs_mode = false; // 0 = incremental; 1 = absolute

// our feedrate variables.
static float feedrate = 0.0;
static uint32_t feedrate_micros = 0;

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
			Serial.println( "Grbl 1.0" );
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
	if ( command == NULL )
	{
		// TODO: Something bad!
	}

	// the character / means delete block... used for comments and stuff.
	if ( command[ 0 ] == '/' )
	{
		Serial.println( "ok" );
		return;
	}

	// init baby!
	Commands::FloatPoint fp;
	fp.x = 0.0;
	fp.y = 0.0;
	fp.z = 0.0;

	uint8_t code = 0;

	// did we get a gcode?
	if ( !StringContains( '$', command, commandLength ) && ( StringContains( 'G', command, commandLength ) || StringContains( 'X', command, commandLength ) || StringContains( 'Y', command, commandLength ) || StringContains( 'Z', command, commandLength ) ) )
	{
		// which one?
		code = ( int ) ExtractNumericPayload( 'G', command, commandLength );

		// Get co-ordinates if required by the code type given
		switch ( code )
		{
		case 0:
		case 1:
		case 2:
		case 3:
			if ( abs_mode )
			{
				// we do it like this to save time. makes curves better.
				// eg. if only x and y are specified, we dont have to waste time looking up z.
				if ( StringContains( 'X', command, commandLength ) )
					fp.x = ExtractNumericPayload( 'X', command, commandLength );
				else
					fp.x = current_units.x;

				if ( StringContains( 'Y', command, commandLength ) )
					fp.y = ExtractNumericPayload( 'Y', command, commandLength );
				else
					fp.y = current_units.y;

				if ( StringContains( 'Z', command, commandLength ) )
					fp.z = ExtractNumericPayload( 'Z', command, commandLength );
				else
					fp.z = current_units.z;
			}
			else
			{
				fp.x = ExtractNumericPayload( 'X', command, commandLength ) + current_units.x;
				fp.y = ExtractNumericPayload( 'Y', command, commandLength ) + current_units.y;
				fp.z = ExtractNumericPayload( 'Z', command, commandLength ) + current_units.z;
			}

			break;
		}
		// do something!
		switch ( code )
		{
		// Rapid Positioning
		// Linear Interpolation
		// these are basically the same thing.
		case 0:
		case 1:
			// set our target.
			set_target( fp.x, fp.y, fp.z );
			// do we have a set speed?
			if ( StringContains( 'G', command, commandLength ) )
			{
				// adjust if we have a specific feedrate.
				if ( code == 1 )
				{
					// how fast do we move?
					feedrate = ExtractNumericPayload( 'F', command, commandLength );
					if ( feedrate > 0 )
						feedrate_micros = calculate_feedrate_delay( feedrate );
					// nope, no feedrate
					else
						feedrate_micros = getMaxSpeed();
				}
				// use our max for normal moves.
				else
					feedrate_micros = getMaxSpeed();
			}
			// nope, just coordinates!
			else
			{
				// do we have a feedrate yet?
				if ( feedrate > 0 )
					feedrate_micros = calculate_feedrate_delay( feedrate );
				// nope, no feedrate
				else
					feedrate_micros = getMaxSpeed();
			}

			// finally move.
			dda_move( feedrate_micros );
			break;

		// Dwell
		case 4:
			delay( ( int ) ExtractNumericPayload( 'P', command, commandLength ) );
			break;

		// go home.
		case 28:
			set_target( 0.0, 0.0, 0.0 );
			goto_machine_zero();
			break;

		// go home via an intermediate point.
		case 30:
			fp.x = ExtractNumericPayload( 'X', command, commandLength );
			fp.y = ExtractNumericPayload( 'Y', command, commandLength );
			fp.z = ExtractNumericPayload( 'Z', command, commandLength );

			// set our target.
			if ( abs_mode )
			{
				if ( !StringContains( 'X', command, commandLength ) )
					fp.x = current_units.x;
				if ( !StringContains( 'Y', command, commandLength ) )
					fp.y = current_units.y;
				if ( !StringContains( 'Z', command, commandLength ) )
					fp.z = current_units.z;

				set_target( fp.x, fp.y, fp.z );
			}
			else
				set_target( current_units.x + fp.x, current_units.y + fp.y, current_units.z + fp.z );

			// go there.
			dda_move( getMaxSpeed() );

			// go home.
			set_target( 0.0, 0.0, 0.0 );
			goto_machine_zero();
			break;

		// Absolute Positioning
		case 90:
			abs_mode = true;
			break;

		// Incremental Positioning
		case 91:
			abs_mode = false;

			break;

		// Set as home
		case 92:
			set_position( 0.0, 0.0, 0.0 );
			break;

			/*
						//Inverse Time Feed Mode
						case 93:

						break;  //TODO: add this

						//Feed per Minute Mode
						case 94:

						break;  //TODO: add this
			*/

		default:
			Serial.print( "huh? G" );
			Serial.println( code, DEC );
		}
	}
	if ( StringContains( 'M', command, commandLength ) )
	{
		code = ExtractNumericPayload( 'M', command, commandLength );
		switch ( code )
		{
		// TODO: this is a bug because ExtractNumericPayload returns 0.  gotta fix that.
		case 0:
			break;

		default:
			Serial.print( "Huh? M" );
			Serial.println( code );
		}
	}
	// tell our host we're done.
	if ( code == 0 && commandLength == 1 )
	{
		Serial.println( "start" );
	}
	else
	{
		Serial.println( "ok" );
	}
	//	Serial.println(line, DEC);
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
