#ifndef PROCESS_STRING_H_
#define PROCESS_STRING_H_

#include "common.h"

// our point structure to make things nice.
struct LongPoint
{
	long x;
	long y;
	long z;
};

struct FloatPoint
{
	float x;
	float y;
	float z;
};

#define COMMAND_SIZE 128
extern char commands[COMMAND_SIZE];
extern uint8_t serial_count;

extern FloatPoint current_units;
extern FloatPoint target_units;
extern FloatPoint delta_units;

extern FloatPoint current_steps;
extern FloatPoint target_steps;
extern FloatPoint delta_steps;

// our direction vars
extern byte x_direction;
extern byte y_direction;
extern byte z_direction;

extern float x_units;
extern float y_units;
extern float z_units;
extern float curve_section;

void init_process_string();
void process_string(char instruction[], int size);
bool has_command(char key, char instruction[], int string_size);
double search_string(char key, char instruction[], int string_size);

#endif
