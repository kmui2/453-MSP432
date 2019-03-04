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
		// UARTs
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
		


    /* Selecting 3.2 and P3.3 in UART mode */
    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P3,
            GPIO_PIN1 | GPIO_PIN2 | GPIO_PIN3, GPIO_PRIMARY_MODULE_FUNCTION);


    /* Configuring UART Module */
    UART_initModule(EUSCI_A2_BASE, &uartConfig_9600);

    /* Enable UART module */
    UART_enableModule(EUSCI_A2_BASE);



    /* Selecting 9.6 and P9.7 in UART mode */
    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P9,
            GPIO_PIN5 | GPIO_PIN6 | GPIO_PIN7, GPIO_PRIMARY_MODULE_FUNCTION);

    /* Configuring UART Module */
    UART_initModule(EUSCI_A3_BASE, &uartConfig_9600);

    /* Enable UART module */
    UART_enableModule(EUSCI_A3_BASE);



		/////////////////
		// Outputs
		/////////////////

    GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN0);
    GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN0);
	


    /* Enabling SRAM Bank Retention */
    SysCtl_enableSRAMBankRetention(SYSCTL_SRAM_BANK1);
    
    /* Enabling interrupts */
    UART_enableInterrupt(EUSCI_A0_BASE, EUSCI_A_UART_RECEIVE_INTERRUPT);
    Interrupt_enableInterrupt(INT_EUSCIA0);
		
    UART_enableInterrupt(EUSCI_A1_BASE, EUSCI_A_UART_RECEIVE_INTERRUPT);
    Interrupt_enableInterrupt(INT_EUSCIA2);
		
    UART_enableInterrupt(EUSCI_A3_BASE, EUSCI_A_UART_RECEIVE_INTERRUPT);
    Interrupt_enableInterrupt(INT_EUSCIA3);
		
		
    Interrupt_enableSleepOnIsrExit();
    Interrupt_enableMaster();   
		
		printDebug("Initialized\n");

    /* Going to LPM3 */
    while (1)
    {
        PCM_gotoLPM0();
    }
}



/* EUSCI A1 UART ISR - Toggles LED */
void EUSCIA2_IRQHandler(void)
{
		uint32_t status = UART_getEnabledInterruptStatus(EUSCI_A2_BASE);
    printDebug("A2 Received\n");
		
    UART_clearInterruptFlag(EUSCI_A2_BASE, status);

    if(status & EUSCI_A_UART_RECEIVE_INTERRUPT_FLAG)
    {
        char data = (char) UART_receiveData(EUSCI_A2_BASE);

        if (data == 'n') {

          GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN0);
          printDebug("LED is on.");
        } else {
          GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN0);
          printDebug("LED is off");
        }
    }

}


/* EUSCI A0 UART ISR - Reroutes serial data from PUTTY A0 to the UART the Bluetooth module is connected A3 then to A2 where the receiver Bluetooth module is connected. */
void EUSCIA0_IRQHandler(void)
{
    uint32_t status = UART_getEnabledInterruptStatus(EUSCI_A0_BASE);

    UART_clearInterruptFlag(EUSCI_A0_BASE, status);

    if(status & EUSCI_A_UART_RECEIVE_INTERRUPT_FLAG)
    {
        char data = reverse(UART_receiveData(EUSCI_A0_BASE));
				
			printDebug("A3 transmitted: ");
			printDebug(&data);
				UART_transmitData(EUSCI_A3_BASE, data);
    }

}



