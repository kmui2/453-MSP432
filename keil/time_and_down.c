/* DriverLib Includes */
#include "debug.h"
#include "driverlib.h"
#include "time_and_down.h"
#include "cmd.h"
#include "seven_segment.h"
#include "stepper.h"

/* Standard Includes */
#include <stdint.h>

#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define min(a, b) ((a) < (b) ? (a) : (b))

const Timer_A_ContinuousModeConfig continuousModeConfig =
{
        TIMER_A_CLOCKSOURCE_ACLK,           // ACLK Clock Source
        TIMER_A_CLOCKSOURCE_DIVIDER_1,      // ACLK/1 = 32.768khz
        TIMER_A_TAIE_INTERRUPT_ENABLE,      // Enable Overflow ISR
        TIMER_A_DO_CLEAR                    // Clear Counter
};

volatile uint8_t quarter = 1;
volatile uint8_t minutes = 15;
volatile uint8_t seconds = 0;
uint8_t down = 1;
uint8_t distance = 10;
uint32_t player1_score = 0;
uint32_t player2_score = 0;
volatile bool halftime = false; 
volatile bool game_over = false;
volatile bool timerStarted = false;

uint32_t getPlayer1Score() {
	return player1_score;
}

void setPlayer1Score(uint32_t new_score) {
	player1_score = new_score;
	setSevenSegmentDisplay1(player1_score);
}

void incrementPlayer1ScoreBy(uint32_t score) {
	player1_score += score;
	setSevenSegmentDisplay1(player1_score);
}

uint32_t getPlayer2Score() {
	return player2_score;
}

void setPlayer2Score(uint32_t new_score) {
	player2_score = new_score;
	setSevenSegmentDisplay1(player2_score);
}

void incrementPlayer2ScoreBy(uint32_t score) {
	player2_score += score;
	setSevenSegmentDisplay1(player2_score);
}

void decrementDistanceBy(uint8_t d) {
	distance -= d;
}

void initTimeAndDown(void) {

		/////////////////
		// Timers
		/////////////////

    /* Starting and enabling ACLK (32kHz) */
    CS_setReferenceOscillatorFrequency(CS_REFO_128KHZ);
    CS_initClockSignal(CS_ACLK, CS_REFOCLK_SELECT, CS_CLOCK_DIVIDER_4);

    /* Configuring Continuous Mode */
    Timer_A_configureContinuousMode(TIMER_A0_BASE, &continuousModeConfig);

}

void resetTimeAndDown(bool offense) {
	setDown(1);
	setDistance(min(offense ? 100 - getYardage() : getYardage(), 10));
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
void setQuarter(uint8_t newQuarter) {
	quarter = newQuarter;
}

void resetTime(void) {
	minutes = 15;
	seconds = 0;
};
void startTime(void) {
	// Start timer
		if (timerStarted) {
			return;
		}
		timerStarted = true;
    Interrupt_enableInterrupt(INT_TA0_N);
    /* Starting the Timer_A0 in continuous mode */
    Timer_A_startCounter(TIMER_A0_BASE, TIMER_A_CONTINUOUS_MODE);
};
void pauseTime(void) {
	if (!timerStarted) {
		return;
	}
	timerStarted = false;
	// FIXME: maybe need a variable to keep track of whether timer stopped
	Interrupt_disableInterrupt(INT_TA0_N);
	// Stop timer
	Timer_A_stopTimer(TIMER_A0_BASE);
};
void decrementTime(void) {
	if (minutes == 0 && seconds == 0) {
		// No more time left
		switch (quarter) {
			case 1:
			case 3:
				quarter++;
				resetTime();
				break;
			case 2:
				quarter++;
				resetTime();
				halftime = true;
				pauseTime();
				break;
			case 4:
				quarter++;
				resetTime();
				game_over = true;
				pauseTime();
				break;
		}
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
void setDown(uint8_t newDown) {
	down = newDown;
}

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


int dec = 0;

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
    // GPIO_toggleOutputOnPin(GPIO_PORT_P2, GPIO_PIN1);
    // GPIO_toggleOutputOnPin(GPIO_PORT_P2, GPIO_PIN2);
		//message = "My Debug Message";
		// printDebug(message);
	
	// if (seconds == 5) {
	// 	pauseTime();
	// }
	decrementTime();
	
	dec = (dec + 1) % 100;
	setSevenSegmentDisplay1(dec);
	setSevenSegmentDisplay2(99 - dec);

	sprintf(cmd, "TIME %i %i %i %i %i", quarter, minutes, seconds, down, distance);
	sendCmd(cmd);
}

