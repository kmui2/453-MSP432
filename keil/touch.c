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

#define DETECTION_STATUS_REG                 4

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
    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P1,
            GPIO_PIN6 + GPIO_PIN7, GPIO_PRIMARY_MODULE_FUNCTION);

    /* Initializing I2C Master to SMCLK at 100khz with no autostop */
    I2C_initMaster(EUSCI_B0_BASE, &i2cConfig);

    /* Specify slave address */
    I2C_setSlaveAddress(EUSCI_B0_BASE, SLAVE_ADDRESS);

    /* Enable I2C Module to start operations */
    I2C_enableModule(EUSCI_B0_BASE);
	
    Interrupt_enableInterrupt(INT_EUSCIB0);

    // enable RX interrupts
    I2C_enableInterrupt(EUSCI_B0_BASE, EUSCI_B_I2C_RECEIVE_INTERRUPT0);
}

volatile bool stopPolling = false;

void pollTouch(void)
{
	while (!stopPolling) {
		/* Making sure the last transaction has been completely sent out */
		while (I2C_masterIsStopSent(EUSCI_B0_BASE));

		I2C_masterSendMultiByteStart(EUSCI_B0_BASE, SLAVE_ADDRESS << 1);
		I2C_masterSendMultiByteFinish(EUSCI_B0_BASE, DETECTION_STATUS_REG);
		I2C_masterSendMultiByteStart(EUSCI_B0_BASE, (SLAVE_ADDRESS << 1) | 0x1); 
		uint8_t RXData = I2C_masterReceiveMultiByteFinish(EUSCI_B0_BASE);

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

    status = I2C_getEnabledInterruptStatus(EUSCI_B0_BASE);
    I2C_clearInterruptFlag(EUSCI_B0_BASE, status);

    // /* Receives bytes into the receive buffer. If we have received all bytes,
    //  * send a STOP condition */
    // if (status & EUSCI_B_I2C_RECEIVE_INTERRUPT0)
    // {
    //     // One-byte Read
    //     RXData = I2C_masterReceiveSingle(EUSCI_B0_BASE);
    //     // GPIO_toggleOutputOnPin(GPIO_PORT_P2, GPIO_PIN1);
    // }
}


