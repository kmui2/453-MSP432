/* DriverLib Includes */
#include "debug.h"
#include "driverlib.h"
#include "time_and_down.h"
#include "cmd.h"

/* Standard Includes */
#include <stdint.h>

#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

const Timer_A_ContinuousModeConfig continuousModeConfig =
{
        TIMER_A_CLOCKSOURCE_ACLK,           // ACLK Clock Source
        TIMER_A_CLOCKSOURCE_DIVIDER_1,      // ACLK/1 = 32.768khz
        TIMER_A_TAIE_INTERRUPT_ENABLE,      // Enable Overflow ISR
        TIMER_A_DO_CLEAR                    // Clear Counter
};

uint8_t quarter = 1;
uint8_t minutes = 15;
uint8_t seconds = 0;
uint8_t down = 1;
uint8_t distance = 10;

void initTimeAndDown(void) {

		/////////////////
		// Timers
		/////////////////

    /* Starting and enabling ACLK (32kHz) */
    CS_setReferenceOscillatorFrequency(CS_REFO_128KHZ);
    CS_initClockSignal(CS_ACLK, CS_REFOCLK_SELECT, CS_CLOCK_DIVIDER_4);

    /* Configuring Continuous Mode */
    Timer_A_configureContinuousMode(TIMER_A0_BASE, &continuousModeConfig);

    /* Enabling interrupts and going to sleep */
    Interrupt_enableSleepOnIsrExit();
    Interrupt_enableInterrupt(INT_TA0_N);

}

uint8_t getQuarter(void) {
	return quarter;
};

void incrementQuarter(void) {
	quarter++;
};
void resetQuarter(void) {
	quarter = 1;
};

void resetTime(void) {
	minutes = 15;
	seconds = 0;
};
void startTime(void) {
	// Start timer

    /* Starting the Timer_A0 in continuous mode */
    Timer_A_startCounter(TIMER_A0_BASE, TIMER_A_CONTINUOUS_MODE);
};
void pauseTime(void) {
	// Stop timer
	Timer_A_stopTimer(TIMER_A0_BASE);
};
void decrementTime(void) {
	if (minutes == 0 && seconds == 0) {
		// No more time left
	} else if (minutes > 0 && seconds == 0) {
		minutes--;
		seconds = 59;
	} else {
		seconds--;
	}
}

uint8_t getDown(void) {
	return down;
};
void incrementDown(void) {
	down++;
};
void resetDown(void) {
	down = 1;
};

uint8_t getDistance(void) {
	return distance;
};
void setDistance(uint8_t new_distance) {
	distance = new_distance;
};
void resetDistance(void) {
	// Should take min from goal and 10.
	distance = 10;
};



//******************************************************************************
//
//This is the TIMERA interrupt vector service routine.
//
//******************************************************************************
void TA0_N_IRQHandler(void)
{
	char cmd[100];
		//char* message;
    Timer_A_clearInterruptFlag(TIMER_A0_BASE);
    GPIO_toggleOutputOnPin(GPIO_PORT_P2, GPIO_PIN1);
    GPIO_toggleOutputOnPin(GPIO_PORT_P2, GPIO_PIN2);
		//message = "My Debug Message";
		// printDebug(message);
	
	// if (seconds == 5) {
	// 	pauseTime();
	// }
	decrementTime();

	sprintf(cmd, "TIME %i %i %i %i %i", quarter, minutes, seconds, down, distance);
	sendCmd(cmd);
}

