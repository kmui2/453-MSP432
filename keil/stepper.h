#ifndef __STEPPER_H__
#define __STEPPER_H__

#include "driverlib.h"

// library interface description

void initStepper(void);

void setStepperSpeed(long whatSpeed);

void step(int steps_to_move);
void movefootball(int8_t yards);

/**
 * Returns true if hits an endzone.
 * The ball will not move in this case.
 **/
bool moveFootballForwardBy(int8_t yards);
void moveFootballToYardage(int yardage);
int getYardage(void);

#endif
