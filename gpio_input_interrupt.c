/* DriverLib Includes */
#include "driverlib.h"
#include <stdio.h>

/* Standard Includes */
#include <stdint.h>

#include <stdbool.h>
#include <string.h>

#define D_PIN GPIO_PIN0
#define C_PIN GPIO_PIN1
#define B_PIN GPIO_PIN2
#define A_PIN GPIO_PIN3

#define DEC_0_HIGH_PINS 0
#define DEC_0_LOW_PINS D_PIN | C_PIN | B_PIN | A_PIN 

#define DEC_1_HIGH_PINS A_PIN
#define DEC_1_LOW_PINS D_PIN | C_PIN | B_PIN

#define DEC_2_HIGH_PINS B_PIN
#define DEC_2_LOW_PINS D_PIN | C_PIN | A_PIN

#define DEC_3_HIGH_PINS B_PIN | A_PIN
#define DEC_3_LOW_PINS D_PIN | C_PIN

#define DEC_4_HIGH_PINS C_PIN
#define DEC_4_LOW_PINS D_PIN | B_PIN | A_PIN

#define DEC_5_HIGH_PINS C_PIN | A_PIN
#define DEC_5_LOW_PINS D_PIN | B_PIN

#define DEC_6_HIGH_PINS C_PIN | B_PIN
#define DEC_6_LOW_PINS D_PIN | A_PIN

#define DEC_7_HIGH_PINS C_PIN | B_PIN | A_PIN
#define DEC_7_LOW_PINS D_PIN

#define DEC_8_HIGH_PINS D_PIN
#define DEC_8_LOW_PINS C_PIN | B_PIN | A_PIN

#define DEC_9_HIGH_PINS D_PIN | A_PIN
#define DEC_9_LOW_PINS C_PIN | B_PIN

const uint_fast8_t decHighPins[] = {
  DEC_0_HIGH_PINS,
  DEC_1_HIGH_PINS,
  DEC_2_HIGH_PINS,
  DEC_3_HIGH_PINS,
  DEC_4_HIGH_PINS,
  DEC_5_HIGH_PINS,
  DEC_6_HIGH_PINS,
  DEC_7_HIGH_PINS,
  DEC_8_HIGH_PINS,
  DEC_9_HIGH_PINS,
};

const uint_fast8_t decLowPins[] = {
  DEC_0_LOW_PINS,
  DEC_1_LOW_PINS,
  DEC_2_LOW_PINS,
  DEC_3_LOW_PINS,
  DEC_4_LOW_PINS,
  DEC_5_LOW_PINS,
  DEC_6_LOW_PINS,
  DEC_7_LOW_PINS,
  DEC_8_LOW_PINS,
  DEC_9_LOW_PINS,
};


/* UART Configuration Parameter. These are the configuration parameters to
 * make the eUSCI A UART module to operate with a 115200 baud rate. These
 * values were calculated using the online calculator that TI provides
 * at:
 * http://software-dl.ti.com/msp430/msp430_public_sw/mcu/msp430/MSP430BaudRateConverter/index.html
 */
const eUSCI_UART_Config uartConfig_115200 =
{
        EUSCI_A_UART_CLOCKSOURCE_SMCLK,          // SMCLK Clock Source
        6,                                      // BRDIV = 13
        8,                                       // UCxBRF = 0
        0,                                      // UCxBRS = 37
        EUSCI_A_UART_NO_PARITY,                  // No Parity
        EUSCI_A_UART_MSB_FIRST,                  // MSB First
        EUSCI_A_UART_ONE_STOP_BIT,               // One stop bit
        EUSCI_A_UART_MODE,                       // UART mode
        EUSCI_A_UART_OVERSAMPLING_BAUDRATE_GENERATION  // Oversampling
};

const eUSCI_UART_Config uartConfig_9600 =
{
        EUSCI_A_UART_CLOCKSOURCE_SMCLK,          // SMCLK Clock Source
        78,                                      // BRDIV = 13
        2,                                       // UCxBRF = 0
        0,                                      // UCxBRS = 37
        EUSCI_A_UART_NO_PARITY,                  // No Parity
        EUSCI_A_UART_MSB_FIRST,                  // MSB First
        EUSCI_A_UART_ONE_STOP_BIT,               // One stop bit
        EUSCI_A_UART_MODE,                       // UART mode
        EUSCI_A_UART_OVERSAMPLING_BAUDRATE_GENERATION  // Oversampling
};


const Timer_A_ContinuousModeConfig continuousModeConfig =
{
        TIMER_A_CLOCKSOURCE_ACLK,           // ACLK Clock Source
        TIMER_A_CLOCKSOURCE_DIVIDER_1,      // ACLK/1 = 32.768khz
        TIMER_A_TAIE_INTERRUPT_ENABLE,      // Enable Overflow ISR
        TIMER_A_DO_CLEAR                    // Clear Counter
};

unsigned char reverse(unsigned char b) {
   b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
   b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
   b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
   return b;
}

void printDebug(char* message) {
		uint16_t i = 0;
		while (message[i] != '\0') {
			while(!(UCA0IFG&UCTXIFG));
			UART_transmitData(EUSCI_A0_BASE, reverse(message[i]));
			i++;
		}
}


int main(void)
{
    volatile uint32_t ii;

    /* Halting the Watchdog */
    WDT_A_holdTimer();
	
	
		/////////////////
		// UART
		/////////////////

    /* Selecting P1.2 and P1.3 in UART mode */
    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P1,
            GPIO_PIN1 | GPIO_PIN2 | GPIO_PIN3, GPIO_PRIMARY_MODULE_FUNCTION);

    /* Setting DCO to 12MHz */
    CS_setDCOCenteredFrequency(CS_DCO_FREQUENCY_12);

    /* Configuring UART Module */
    UART_initModule(EUSCI_A0_BASE, &uartConfig_9600);

    /* Enable UART module */
    UART_enableModule(EUSCI_A0_BASE);

	
	
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

    /* Starting the Timer_A0 in continuous mode */
    Timer_A_startCounter(TIMER_A0_BASE, TIMER_A_CONTINUOUS_MODE);
	
	
		/////////////////
		// Outputs
		/////////////////

    GPIO_setAsOutputPin(GPIO_PORT_P4, D_PIN | C_PIN | B_PIN | A_PIN);
    GPIO_setOutputLowOnPin(GPIO_PORT_P4, D_PIN | C_PIN | B_PIN | A_PIN);	

    /* Enabling SRAM Bank Retention */
    SysCtl_enableSRAMBankRetention(SYSCTL_SRAM_BANK1);
    
    /* Enabling MASTER interrupts */
    Interrupt_enableMaster();   

    /* Going to LPM3 */
    while (1)
    {
        PCM_gotoLPM0();
    }
}

int dec = 0;

//******************************************************************************
//
//This is the TIMERA interrupt vector service routine.
//
//******************************************************************************
void TA0_N_IRQHandler(void)
{
		char message[42];
    Timer_A_clearInterruptFlag(TIMER_A0_BASE);
    GPIO_setOutputHighOnPin(GPIO_PORT_P4, decHighPins[dec]);
    GPIO_setOutputLowOnPin(GPIO_PORT_P4, decLowPins[dec]);
		sprintf(message, "%d", dec);
    printDebug("Sending ");
		printDebug(message);
    printDebug("\n");
    dec = (dec + 1) % 10;
}

