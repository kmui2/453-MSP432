/* DriverLib Includes */
#include "debug.h"
#include "stepper.h"
#include "driverlib.h"
#include <stdlib.h>

/* Standard Includes */
#include <stdint.h>
#include <stdio.h>

#include <stdbool.h>
#include <string.h>
#include "systick.h"

#define HIGH 1
#define LOW 0


 int step_number = 0;    // which step the motor is on
 int direction = 0;      // motor direction
 int last_step_time = 0; // time stamp in us of the last step taken
 int number_of_steps = 4096; // total number of steps for this motor
 float step_delay = 0;
 int i = 0;

int steps_left = 0;

volatile int8_t absolute_yardage = -10;

  // Arduino pins for the motor control connection:
	// TODO: use #define
	#define MOTOR_PORT GPIO_PORT_P5
	#define MOTOR_PIN_1 GPIO_PIN0
	#define MOTOR_PIN_2 GPIO_PIN6
	#define MOTOR_PIN_3 GPIO_PIN1
	#define MOTOR_PIN_4 GPIO_PIN7


int getYardage() {
  return absolute_yardage;
}

void initStepper() {
    GPIO_setAsOutputPin(MOTOR_PORT, MOTOR_PIN_1);
    GPIO_setOutputLowOnPin(MOTOR_PORT, MOTOR_PIN_1);
		
    GPIO_setAsOutputPin(MOTOR_PORT, MOTOR_PIN_2);
    GPIO_setOutputLowOnPin(MOTOR_PORT, MOTOR_PIN_2);
		
    GPIO_setAsOutputPin(MOTOR_PORT, MOTOR_PIN_3);
    GPIO_setOutputLowOnPin(MOTOR_PORT, MOTOR_PIN_3);
		
    GPIO_setAsOutputPin(MOTOR_PORT, MOTOR_PIN_4);
    GPIO_setOutputLowOnPin(MOTOR_PORT, MOTOR_PIN_4);

    /* Configuring Timer32 to 128000 (1s) of MCLK in periodic mode */
    Timer32_initModule(TIMER32_BASE, TIMER32_PRESCALER_1, TIMER32_32BIT,
            TIMER32_PERIODIC_MODE);

    /* Enabling interrupts */	
    Interrupt_enableInterrupt(INT_T32_INT1);
    Timer32_enableInterrupt(TIMER32_BASE);
};

float reload;

/*
* TODO: use this speed for a timer
 * Moves the motor steps_to_move steps.  If the number is negative,
 * the motor moves in the reverse direction.
 * 
 * whatSpeed is in RPM
 * stop_delay is in seconds
 */
void setStepperSpeed(long whatSpeed) {
  // FIXME: whatSpeed is not used.
	//step_delay = 60L / (float) number_of_steps / whatSpeed;
	step_delay = 0.15;
	reload = step_delay * 128000;
  Timer32_setCount(TIMER32_BASE, (int) reload);
}

void digitalWrite(uint_fast16_t pin, int level) {
	if (level == HIGH) {
    GPIO_setOutputHighOnPin(MOTOR_PORT, pin);
	} else if (level == LOW) {
    GPIO_setOutputLowOnPin(MOTOR_PORT, pin);
	}
}

void stepMotor(int thisStep)
{
	switch (thisStep) {
		case 0:  // 0001
			digitalWrite(MOTOR_PIN_1, LOW);
			digitalWrite(MOTOR_PIN_2, LOW);
			digitalWrite(MOTOR_PIN_3, LOW);
			digitalWrite(MOTOR_PIN_4, HIGH);
		break;
		case 1:  // 0011
			digitalWrite(MOTOR_PIN_1, LOW);
			digitalWrite(MOTOR_PIN_2, LOW);
			digitalWrite(MOTOR_PIN_3, HIGH);
			digitalWrite(MOTOR_PIN_4, HIGH);
		break;
		case 2:  //0010
			digitalWrite(MOTOR_PIN_1, LOW);
			digitalWrite(MOTOR_PIN_2, LOW);
			digitalWrite(MOTOR_PIN_3, HIGH);
			digitalWrite(MOTOR_PIN_4, LOW);
		break;
		case 3:  //0110
			digitalWrite(MOTOR_PIN_1, LOW);
			digitalWrite(MOTOR_PIN_2, HIGH);
			digitalWrite(MOTOR_PIN_3, HIGH);
			digitalWrite(MOTOR_PIN_4, LOW);
		break;
		case 4:  //0100
			digitalWrite(MOTOR_PIN_1, LOW);
			digitalWrite(MOTOR_PIN_2, HIGH);
			digitalWrite(MOTOR_PIN_3, LOW);
			digitalWrite(MOTOR_PIN_4, LOW);
		break;
		case 5:  //1100
			digitalWrite(MOTOR_PIN_1, HIGH);
			digitalWrite(MOTOR_PIN_2, HIGH);
			digitalWrite(MOTOR_PIN_3, LOW);
			digitalWrite(MOTOR_PIN_4, LOW);
		break;
		case 6:  //1001
			digitalWrite(MOTOR_PIN_1, HIGH);
			digitalWrite(MOTOR_PIN_2, LOW);
			digitalWrite(MOTOR_PIN_3, LOW);
			digitalWrite(MOTOR_PIN_4, HIGH);
		break;
		case 7:  //1001
			digitalWrite(MOTOR_PIN_1, HIGH);
			digitalWrite(MOTOR_PIN_2, LOW);
			digitalWrite(MOTOR_PIN_3, LOW);
			digitalWrite(MOTOR_PIN_4, HIGH);
		break;
	}
	
}


void step(int steps_to_move) {
  bool is_running = steps_left > 0;
  steps_left = abs(steps_to_move);  // how many steps to take
	
  // determine direction based on whether steps_to_move is + or -:	  

  if (steps_to_move > 0) { direction = 1; }
  if (steps_to_move < 0) { direction = 0; }
	

  // only start timer when it hasn't started
		if (!is_running) {
			Timer32_startTimer(TIMER32_BASE, false);
		}

}
void movefootball(int8_t yards){
	int steps;
	steps=yards*409.6; 
	step(steps);
	while(steps_left > 0) {
        PCM_gotoLPM0();
	}
}

bool moveFootballForwardBy(int8_t yards) {
  if (absolute_yardage + yards <= 0 || absolute_yardage + yards >= 100) {
    return true;
  }
  absolute_yardage = absolute_yardage + yards;
	// TODO: add me back
  movefootball(yards);
  return false;
};

void moveFootballToYardage(int yardage) {
  int yardsToMove = yardage - absolute_yardage;
  moveFootballForwardBy(yardsToMove);
}



/* Timer32 ISR */
void T32_INT1_IRQHandler(void)
{
	steps_left--;
    Timer32_clearInterruptFlag(TIMER32_BASE); //clear the flag
    
  if (steps_left <= 0)
  {
    // FIXME: Halt doesn't work
		digitalWrite(MOTOR_PIN_1, LOW);
		digitalWrite(MOTOR_PIN_2, LOW);
		digitalWrite(MOTOR_PIN_3, LOW);
		digitalWrite(MOTOR_PIN_4, LOW);
		Timer32_haltTimer(TIMER32_BASE);
		
  }
    // increment or decrement the step number,
    // depending on direction:
    if (direction == 1)
    {
      step_number++;
      if (step_number == number_of_steps) {
        step_number = 0;
      }
    }
    else
    {
      if (step_number == 0) {
        step_number = number_of_steps;
      }
      step_number--;
    }
    // decrement the steps left:
    stepMotor(step_number % 7);

}

