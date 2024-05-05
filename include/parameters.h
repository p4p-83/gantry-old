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
 * Maximum feed rates
 */
#define PARAMETERS_MAX_XY_MM_PER_SEC 20
#define PARAMETERS_MAX_Z_MM_PER_SEC	 20

/*
 * Polarity of limit switches
 */

#define PARAMETERS_LIMIT_SWITCHES_ACTIVE_LOW true

/*
 * Idle sleep parameters
 */
#define PARAMETERS_SLEEP_IF_IDLE				0
#define PARAMETERS_IDLE_ITERATIONS_BEFORE_SLEEP 10000

#endif
