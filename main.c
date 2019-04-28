/* DriverLib Includes */
#include "debug.h"
#include "driverlib.h"
#include "stepper.h"
#include "time_and_down.h"
#include "seven_segment.h"
#include "helpers.h"

/* Standard Includes */
#include <stdint.h>
#include <stdio.h>

#include <stdbool.h>
#include <string.h>

int main(void)
{
    volatile uint32_t ii;
    
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

    /* Halting the Watchdog */
    WDT_A_holdTimer();
	
	
    /* Setting DCO to 12MHz */
    CS_setDCOCenteredFrequency(CS_DCO_FREQUENCY_12);

    /////////////////
    // UART
    /////////////////

    initUartDebug();
	
    /* Selecting 3.2 and P3.3 in UART mode */
    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P3,
            GPIO_PIN1 | GPIO_PIN2 | GPIO_PIN3, GPIO_PRIMARY_MODULE_FUNCTION);
	
    /* Configuring UART Module */
    UART_initModule(EUSCI_A2_BASE, &uartConfig_9600);

    /* Enable UART module */
    UART_enableModule(EUSCI_A2_BASE);
		
		
    UART_enableInterrupt(EUSCI_A2_BASE, EUSCI_A_UART_RECEIVE_INTERRUPT);
    Interrupt_enableInterrupt(INT_EUSCIA2);


    /////////////////
    // Outputs
    /////////////////

    initStepper();
    setStepperSpeed(15);
    initSevenSegment();
    initTimeAndDown();

    GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN0);
    GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN0);
		
    GPIO_setAsOutputPin(GPIO_PORT_P2, GPIO_PIN0);
    GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN0);
		
    GPIO_setAsOutputPin(GPIO_PORT_P2, GPIO_PIN1);
    GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN1);
		
    GPIO_setAsOutputPin(GPIO_PORT_P2, GPIO_PIN2);
    GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN2);
	
	
		/////////////////
		// Inputs
		/////////////////
	
    /* Configuring P1.1 as an input and enabling interrupts */
    GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P1, GPIO_PIN1);
    GPIO_clearInterruptFlag(GPIO_PORT_P1, GPIO_PIN1);
    GPIO_enableInterrupt(GPIO_PORT_P1, GPIO_PIN1);
    Interrupt_enableInterrupt(INT_PORT1);

    /* Configuring P1.4 as an input and enabling interrupts */
    GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P1, GPIO_PIN4);
    GPIO_clearInterruptFlag(GPIO_PORT_P1, GPIO_PIN4);
    GPIO_enableInterrupt(GPIO_PORT_P1, GPIO_PIN4);

    /* Enabling SRAM Bank Retention */
    SysCtl_enableSRAMBankRetention(SYSCTL_SRAM_BANK1);
    
    /* Enabling MASTER interrupts */
    Interrupt_enableMaster();   

    step(2048);
    startTime();
    /* Going to LPM3 */
    while (1)
    {
        PCM_gotoLPM0();
    }
}



/* EUSCI A2 UART ISR - Toggles LED */
void EUSCIA2_IRQHandler(void)
{
		uint32_t status = UART_getEnabledInterruptStatus(EUSCI_A2_BASE);
		
    UART_clearInterruptFlag(EUSCI_A2_BASE, status);

    if(status & EUSCI_A_UART_RECEIVE_INTERRUPT_FLAG)
    {
        uint8_t data = reverse(UART_receiveData(EUSCI_A2_BASE));
				char message[100];
				sprintf(message, "detection_status=%d\n", data);
				printDebug(message);
    }
}


/* GPIO ISR */
void PORT1_IRQHandler(void)
{
    uint32_t status;

    status = GPIO_getEnabledInterruptStatus(GPIO_PORT_P1);
    GPIO_clearInterruptFlag(GPIO_PORT_P1, status);

    /* Toggling the output on the LED */
    if(status & GPIO_PIN1)
    {
        GPIO_toggleOutputOnPin(GPIO_PORT_P1, GPIO_PIN0);
    }

    if(status & GPIO_PIN4)
    {
        GPIO_toggleOutputOnPin(GPIO_PORT_P2, GPIO_PIN0);
    }

}
