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

extern Steppers::Direction x_direction;
extern Steppers::Direction y_direction;
extern Steppers::Direction z_direction;

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
 * @brief Set the current units position of the head.
 *
 * @param xUnits The current x-axis position of the head in units.
 * @param yUnits The current y-axis position of the head in units.
 * @param zUnits The current z-axis position of the head in units.
 */
void SetPosition( float xUnits, float yUnits, float zUnits );
/**
 * @brief Set the target units position of the head.
 *
 * @param xUnits The target x-axis position of the head in units.
 * @param yUnits The target y-axis position of the head in units.
 * @param zUnits The target z-axis position of the head in units.
 */
void SetTarget( float xUnits, float yUnits, float zUnits );

/**
 * @brief Calculate the unit and step deltas between the current and target positions.
 *
 * @note This function also `digitalWrite()`s the necessary logic level
 * to each stepper's direction pin for translation towards the target position.
 *
 */
void CalculateDeltas( void );
/**
 * @brief Calculate the delay for which to delay between taking each step
 * to achieve the desired feed rate.
 *
 * @param mmPerSec The desired feed rate in millimetres per second.
 * @return uint32_t The minimum rate delay in microseconds.
 */
uint32_t CalculateRateDelayMicroseconds( float mmPerSec );

/**
 * @brief Move to the zero position.
 */
void MoveToZero( void );
/**
 * @brief Move to the maximum position (ie hit the limit switch).
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
