#ifndef __STEPPER_H__
#define __STEPPER_H__

#include "driverlib.h"

// library interface description

void initStepper(void);

void setStepperSpeed(long whatSpeed);

void step(int steps_to_move);
void movefootball(int inches);
void moveFootballToYardage(int yardage);
int getYardage(void);

#endif
