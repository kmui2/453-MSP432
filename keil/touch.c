#include "debug.h"
#include "driverlib.h"
#include "stepper.h"
#include "time_and_down.h"

/* Standard Includes */
#include <stdint.h>

#include <stdbool.h>
#include <string.h>

/* Slave Address for I2C Slave */
// There is one preset I2C address of 0x12. This is not changeable
#define SLAVE_ADDRESS 0x12

#define DETECTION_STATUS_REG 4

#define TOUCH_PORT GPIO_PORT_P1
#define TOUCH_SDA GPIO_PIN6
#define TOUCH_SCL GPIO_PIN7
#define TOUCH_EUSCI_BASE EUSCI_B0_BASE
// Remember to change the handler below too.
#define TOUCH_RECEIVE_INTERRUPT EUSCI_B_I2C_RECEIVE_INTERRUPT0

/* I2C Master Configuration Parameter */
const eUSCI_I2C_MasterConfig i2cConfig =
{
        EUSCI_B_I2C_CLOCKSOURCE_SMCLK,          // SMCLK Clock Source
        3000000,                                // SMCLK = 3MHz (default)
        EUSCI_B_I2C_SET_DATA_RATE_100KBPS,      // Desired I2C Clock of 100khz
        0,                                      // No byte counter threshold
        EUSCI_B_I2C_NO_AUTO_STOP                // No Autostop
};

void initTouch(void)
{
    /* Select Port 1 for I2C - Set Pin 6, 7 to input Primary Module Function,
     *   (UCB0SIMO/UCB0SDA, UCB0SOMI/UCB0SCL).
     */
    GPIO_setAsPeripheralModuleFunctionInputPin(TOUCH_PORT,
            TOUCH_SDA + TOUCH_SCL, GPIO_PRIMARY_MODULE_FUNCTION);

    /* Initializing I2C Master to SMCLK at 100khz with no autostop */
    I2C_initMaster(TOUCH_EUSCI_BASE, &i2cConfig);

    /* Specify slave address */
    I2C_setSlaveAddress(TOUCH_EUSCI_BASE, SLAVE_ADDRESS);

    /* Enable I2C Module to start operations */
    I2C_enableModule(TOUCH_EUSCI_BASE);
	
    Interrupt_enableInterrupt(INT_EUSCIB0);

    // enable RX interrupts
    I2C_enableInterrupt(TOUCH_EUSCI_BASE, TOUCH_RECEIVE_INTERRUPT);
}

volatile bool stopPolling = false;

void pollTouch(void)
{
    uint8_t RXData;
	while (!stopPolling) {
		/* Making sure the last transaction has been completely sent out */
		while (I2C_masterIsStopSent(TOUCH_EUSCI_BASE));

		I2C_masterSendMultiByteStart(TOUCH_EUSCI_BASE, SLAVE_ADDRESS << 1);
		I2C_masterSendMultiByteFinish(TOUCH_EUSCI_BASE, DETECTION_STATUS_REG);
		I2C_masterSendMultiByteStart(TOUCH_EUSCI_BASE, (SLAVE_ADDRESS << 1) | 0x1); 
		RXData = I2C_masterReceiveMultiByteFinish(TOUCH_EUSCI_BASE);

		__delay_cycles(300000); // ~100ms pause between transmissions
	}
}


/*******************************************************************************
 * eUSCIB0 ISR. The repeated start and transmit/receive operations happen
 * within this ISR.
 *******************************************************************************/
void EUSCIB0_IRQHandler(void)
{
    uint_fast16_t status;

    status = I2C_getEnabledInterruptStatus(TOUCH_EUSCI_BASE);
    I2C_clearInterruptFlag(TOUCH_EUSCI_BASE, status);
}


