/**
 * @file parameters.h
 * @brief The physical parameters of our gantry system.
 */

#ifndef PARAMETERS_H_
#define PARAMETERS_H_

/*
 * Physical steps per millimetre
 */

#define PARAMETERS_X_STEPS_PER_MM 40
#define PARAMETERS_Y_STEPS_PER_MM 40
#define PARAMETERS_Z_STEPS_PER_MM 40

/*
 * Maximum feedrates
 */
#define PARAMETERS_MAX_XY_RATE 2000
#define PARAMETERS_MAX_Z_RATE  2000

/*
 * Polarity of limit switches
 */

// TODO: what should this be?
#define PARAMETERS_LIMIT_SWITCHES_ACTIVE_LOW 1

#endif
