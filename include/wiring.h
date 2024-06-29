/**
 * @file wiring.h
 * @brief The physical wiring of our gantry system.
 */

#ifndef WIRING_H_
#define WIRING_H_

#define WIRING_UNUSED_PIN -1

/*
 * x-axis
 */

// Port 1
#define WIRING_X_DIRECTION_PIN 3
#define WIRING_X_STEP_PIN	   9

// Port 6
#define WIRING_X_LIMIT_MIN_PIN 13
#define WIRING_X_LIMIT_MAX_PIN 12

#define WIRING_X_ENABLE_PIN	   WIRING_UNUSED_PIN

/*
 * y-axis
 */

// Port 2
#define WIRING_Y_DIRECTION_PIN 10
#define WIRING_Y_STEP_PIN	   11

// Port 3
#define WIRING_Y_LIMIT_MIN_PIN 2
#define WIRING_Y_LIMIT_MAX_PIN 8

#define WIRING_Y_ENABLE_PIN	   WIRING_UNUSED_PIN

/*
 * z-axis
 */

#define WIRING_Z_AXIS_SUPPORTED false

// Port 7
#define WIRING_Z_DIRECTION_PIN WIRING_UNUSED_PIN
#define WIRING_Z_STEP_PIN	   15

#define WIRING_Z_LIMIT_MIN_PIN WIRING_UNUSED_PIN
#define WIRING_Z_LIMIT_MAX_PIN WIRING_UNUSED_PIN

#define WIRING_Z_ENABLE_PIN	   WIRING_UNUSED_PIN

#endif
