/* DriverLib Includes */
#include "driverlib.h"

/* Standard Includes */
#include <stdint.h>

#include <stdbool.h>
#include <string.h>

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

void sendCmd(char* message) {
		uint16_t i = 0;
		while (message[i] != '\0') {
			while(!(UCA0IFG&UCTXIFG));
			UART_transmitData(EUSCI_A0_BASE, reverse(message[i]));
			i++;
		}
		UART_transmitData(EUSCI_A0_BASE, reverse(message[i]));
		UART_transmitData(EUSCI_A0_BASE, reverse('\n'));
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

uint16_t ticks = 0;
#define NUM_SCREENS 5
char* screens[] = { "MAIN", "TIC_TAC_TOE", "TRIVIA", "SIMON_SAYS", "ROCK_PAPER_SCISSORS" };

//******************************************************************************
//
//This is the TIMERA interrupt vector service routine.
//
//******************************************************************************
void TA0_N_IRQHandler(void)
{
    Timer_A_clearInterruptFlag(TIMER_A0_BASE);
    GPIO_toggleOutputOnPin(GPIO_PORT_P2, GPIO_PIN1);
    GPIO_toggleOutputOnPin(GPIO_PORT_P2, GPIO_PIN2);
		sendCmd(screens[ticks % NUM_SCREENS]);
	
		ticks++;
}

