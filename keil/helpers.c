#include "driverlib.h"
#include "helpers.h"
#include "stdlib.h"

// Used for any countdown or delays
volatile bool countdownDone = true;
volatile uint8_t countdownCountLeft = 0;

unsigned char reverse(unsigned char b)
{
   b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
   b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
   b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
   return b;
}

float r2()
{
   float r = (float)rand();
   float res = r / (float)RAND_MAX;
   return res;
}

void delayTimer(uint8_t seconds)
{
   // TODO:

   countdownDone = false;
   countdownCountLeft = seconds * 10;

   Interrupt_enableInterrupt(INT_TA1_0);
   Timer_A_startCounter(TIMER_A1_BASE, TIMER_A_UP_MODE);

   while (!countdownDone)
   {
      PCM_gotoLPM0();
   }

   countdownDone = true;
};

void TA1_0_IRQHandler(void)
{
   MAP_Timer_A_clearCaptureCompareInterrupt(TIMER_A1_BASE,
                                            TIMER_A_CAPTURECOMPARE_REGISTER_0);
   countdownCountLeft--;
   if (countdownCountLeft == 0)
   {
      countdownDone = true;
      Timer_A_stopTimer(TIMER_A1_BASE);
      Interrupt_disableInterrupt(INT_TA1_N);
   }
}
