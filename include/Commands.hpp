#ifndef COMMANDS_H_
#define COMMANDS_H_

#include <stdint.h>

#include "common.h"

namespace Commands
{

struct LongPoint
{
	uint32_t x;
	uint32_t y;
	uint32_t z;
};

struct FloatPoint
{
	float x;
	float y;
	float z;
};

#define COMMAND_MAX_LENGTH				   128
#define COMMAND_NUMERIC_PAYLOAD_MAX_LENGTH 12
#define COMMAND_TERMINATOR				   '\n'

extern char COMMAND_ABSOLUTE_POSITION[];

extern FloatPoint current_units;
extern FloatPoint target_units;
extern FloatPoint delta_units;

extern FloatPoint current_steps;
extern FloatPoint target_steps;
extern FloatPoint delta_steps;

// our direction vars
extern uint8_t x_direction;
extern uint8_t y_direction;
extern uint8_t z_direction;

extern float x_units;
extern float y_units;
extern float z_units;
extern float curve_section;

void ClearCommandBuffer( void );

/**
 * @brief Read a byte from the serial input.
 *
 * @return true if a complete command has been received. \
 * @return false otherwise.
 */
bool ReceiveByte( void );

/**
 * @brief Execute the received command.
 */
void ExecuteReceived( void );
/**
 * @brief Execute a provided command.
 *
 * @param command The command to execute.
 * @param commandLength
 */
void Execute( char *command, size_t commandLength );

}

#endif
