#ifndef STEPPERS_H_
#define STEPPERS_H_

#include <stdint.h>

namespace Steppers
{

enum class Direction : uint8_t
{
	TOWARDS_MIN = 0,
	TOWARDS_MAX = 1,
};

struct Point
{
	uint32_t x;
	uint32_t y;
	uint32_t z;
};

// TODO: Make as many of these private as I can...
// TODO: External API should be through SetCurrentPoint()/SetTargetPoint() ONLY
extern Steppers::Point currentPointMicrometres;
extern Steppers::Point targetPointMicrometres;
extern Steppers::Point deltaMicrometres;

extern Steppers::Point currentPointSteps;
extern Steppers::Point targetPointSteps;
extern Steppers::Point deltaSteps;

extern Steppers::Direction xDirection;
extern Steppers::Direction yDirection;
extern Steppers::Direction zDirection;

/**
 * @brief Initialise the stepper motors.
 */
void Initialise( void );
/**
 * @brief Set the stepper motor enable pins.
 */
void Enable( void );
/**
 * @brief Clear the stepper motor enable pins.
 */
void Disable( void );

/**
 * @brief Get the minimum stepper feed rate delay of the system.
 *
 * The feed rate delay is the time for which to delay before taking each step
 * to achieve the desired feed rate.
 *
 * @return uint32_t The minimum rate delay in microseconds.
 */
uint32_t GetMinRateDelayMicroseconds( void );

/**
 * @brief Set the current point of the head.
 *
 * @param xMicrometres The current x-axis point of the head in micrometres.
 * @param yMicrometres The current y-axis point of the head in micrometres.
 * @param zMicrometres The current z-axis point of the head in micrometres.
 */
void SetCurrentPoint( uint32_t xMicrometres, uint32_t yMicrometres, uint32_t zMicrometres );
/**
 * @brief Set the target point of the head.
 *
 * @param xMicrometres The target x-axis point of the head in micrometres.
 * @param yMicrometres The target y-axis point of the head in micrometres.
 * @param zMicrometres The target z-axis point of the head in micrometres.
 */
void SetTargetPoint( uint32_t xMicrometres, uint32_t yMicrometres, uint32_t zMicrometres );

/**
 * @brief Calculate the micrometre and step deltas between the current and target points.
 *
 * @note This function also `digitalWrite()`s the necessary logic level
 * to each stepper's direction pin for translation towards the target point.
 *
 */
void CalculateDeltas( void );
/**
 * @brief Calculate the delay for which to pause between taking each step
 * to achieve the desired feed rate.
 *
 * @note The unit of distance units per minute is used here to comply with standard G-Code format.
 *
 * @param mmPerMinute The desired feed rate in millimetres per minute.
 * @return uint32_t The minimum rate delay in microseconds.
 */
uint32_t CalculateRateDelayMicroseconds( uint32_t mmPerMinute );

/**
 * @brief Move to the zero point.
 */
void MoveToZero( void );
/**
 * @brief Move to the maximum point (ie hit the limit switch).
 *
 * @param limitPin The pin of the limit switch in the target direction.
 * @param stepperPin The stepper pin of the axis in which to move.
 * @param stepperDirectionPin The stepper direction pin of the axis in which to move.
 * @param direction The direction in which to move.
 */
void MoveToLimit( uint8_t limitPin, uint8_t stepperPin, uint8_t stepperDirectionPin, Steppers::Direction direction );
/**
 * @brief
 *
 * @deprecated Unlikely to be needed.
 *
 * @param micro_delay
 */
void dda_move( long micro_delay );

/**
 * @brief Check whether the target step count has been reached, or a limit switch has been depressed.
 *
 * @param currentSteps The current step count.
 * @param targetSteps The target step count.
 * @param direction The direction in which the head is moving.
 * @param limitMinPin The pin of the minimum limit switch of the active axis.
 * @param limitMaxPin The pin of the maximum limit switch of the active axis.
 * @return true if the target has been reached, ie no further steps should be taken. \
 * @return false otherwise.
 */
bool IsTargetReached( uint32_t currentSteps, uint32_t targetSteps, Steppers::Direction direction, uint8_t limitMinPin, uint8_t limitMaxPin );
}

#endif
