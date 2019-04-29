#include "driverlib.h"
#include "helpers.h"
#include "stdlib.h"


// Used for any countdown or delays
volatile bool countdownDone = true;
volatile uint8_t countdownCountLeft = 0;

unsigned char reverse(unsigned char b) {
   b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
   b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
   b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
   return b;
}

double r2()
{
    return (double)rand() / (double)RAND_MAX ;
}

void delayTimer(uint8_t seconds) {
   countdownDone = false;
   countdownCountLeft = seconds;
   
   Interrupt_enableInterrupt(INT_TA1_N);

   Timer_A_startCounter(TIMER_A0_BASE, TIMER_A_CONTINUOUS_MODE);

   while(!countdownDone) {
      PCM_gotoLPM0();
   }

   countdownDone = true;
};

void TA1_N_IRQHandler(void)
{
   Timer_A_clearInterruptFlag(TIMER_A1_BASE);
   countdownCountLeft--;
   if (countdownCountLeft == 0) {
      countdownDone = true;
	   Timer_A_stopTimer(TIMER_A1_BASE);
      Interrupt_disableInterrupt(INT_TA1_N);
   }
}

