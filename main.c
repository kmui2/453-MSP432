/* DriverLib Includes */
#include "debug.h"
#include "driverlib.h"
#include "stepper.h"
#include "time_and_down.h"
#include "seven_segment.h"
#include "touch.h"

/* Standard Includes */
#include <stdint.h>

#include <stdbool.h>
#include <string.h>

int main(void)
{
    volatile uint32_t ii;

    /* Halting the Watchdog */
    WDT_A_holdTimer();
	
	
    /* Setting DCO to 12MHz */
    CS_setDCOCenteredFrequency(CS_DCO_FREQUENCY_12);

		/////////////////
		// UART
		/////////////////

		initUartDebug();
	
	
		/////////////////
		// Outputs
		/////////////////
	
		initStepper();
    setStepperSpeed(15);
    initTimeAndDown();
    initSevenSegment();
    initTouch();

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
