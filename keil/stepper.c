/* DriverLib Includes */
#include "debug.h"
#include "stepper.h"
#include "driverlib.h"
#include <stdlib.h>

/* Standard Includes */
#include <stdint.h>

#include <stdbool.h>
#include <string.h>

#define HIGH 1
#define LOW 0


 int step_number = 0;    // which step the motor is on
 int direction = 0;      // motor direction
 int last_step_time = 0; // time stamp in us of the last step taken
 int number_of_steps = 2048; // total number of steps for this motor
 float step_delay = 0;

int steps_left = 0;

  // Arduino pins for the motor control connection:
	// TODO: use #define
	#define MOTOR_PORT GPIO_PORT_P5
	#define MOTOR_PIN_1 GPIO_PIN0
	#define MOTOR_PIN_2 GPIO_PIN6
	#define MOTOR_PIN_3 GPIO_PIN1
	#define MOTOR_PIN_4 GPIO_PIN7

void initStepper() {
    GPIO_setAsOutputPin(MOTOR_PORT, MOTOR_PIN_1);
    GPIO_setOutputLowOnPin(MOTOR_PORT, MOTOR_PIN_1);
		
    GPIO_setAsOutputPin(MOTOR_PORT, MOTOR_PIN_2);
    GPIO_setOutputLowOnPin(MOTOR_PORT, MOTOR_PIN_2);
		
    GPIO_setAsOutputPin(MOTOR_PORT, MOTOR_PIN_3);
    GPIO_setOutputLowOnPin(MOTOR_PORT, MOTOR_PIN_3);
		
    GPIO_setAsOutputPin(MOTOR_PORT, MOTOR_PIN_4);
    GPIO_setOutputLowOnPin(MOTOR_PORT, MOTOR_PIN_4);
	
    // /* Setting MCLK to REFO at 128Khz for LF mode */
    // CS_setReferenceOscillatorFrequency(CS_REFO_128KHZ);
    // CS_initClockSignal(CS_MCLK, CS_REFOCLK_SELECT, CS_CLOCK_DIVIDER_1);
		// PCM_setPowerState(PCM_AM_LF_VCORE0);

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
	step_delay = 0.25;
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
		case 0:  // 1010
			digitalWrite(MOTOR_PIN_1, HIGH);
			digitalWrite(MOTOR_PIN_2, LOW);
			digitalWrite(MOTOR_PIN_3, HIGH);
			digitalWrite(MOTOR_PIN_4, LOW);
		break;
		case 1:  // 0110
			digitalWrite(MOTOR_PIN_1, LOW);
			digitalWrite(MOTOR_PIN_2, HIGH);
			digitalWrite(MOTOR_PIN_3, HIGH);
			digitalWrite(MOTOR_PIN_4, LOW);
		break;
		case 2:  //0101
			digitalWrite(MOTOR_PIN_1, LOW);
			digitalWrite(MOTOR_PIN_2, HIGH);
			digitalWrite(MOTOR_PIN_3, LOW);
			digitalWrite(MOTOR_PIN_4, HIGH);
		break;
		case 3:  //1001
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

  // determine direction based on whether steps_to_mode is + or -:
  if (steps_to_move > 0) { direction = 1; }
  if (steps_to_move < 0) { direction = 0; }
  
  // only start timer when it hasn't started
  if (!is_running) {
    Timer32_startTimer(TIMER32_BASE, false);
  }
}


/* Timer32 ISR */
void T32_INT1_IRQHandler(void)
{
    Timer32_clearInterruptFlag(TIMER32_BASE);
    steps_left--;
  if (steps_left <= 0)
  {
    // FIXME: Halt doesn't work
    Timer32_haltTimer(TIMER32_BASE);
		digitalWrite(MOTOR_PIN_1, LOW);
		digitalWrite(MOTOR_PIN_2, LOW);
		digitalWrite(MOTOR_PIN_3, LOW);
		digitalWrite(MOTOR_PIN_4, LOW);
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
    stepMotor(step_number % 4);

}
