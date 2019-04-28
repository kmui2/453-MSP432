/* DriverLib Includes */
#include "driverlib.h"
#include "helpers.h"

/* Standard Includes */
#include <stdint.h>

#include <stdbool.h>
#include <string.h>
#include "debug.h"



void initUartDebug(void) {
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
    /* Selecting P1.2 and P1.3 in UART mode */
    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P1,
            GPIO_PIN1 | GPIO_PIN2 | GPIO_PIN3, GPIO_PRIMARY_MODULE_FUNCTION);

    /* Configuring UART Module */
    UART_initModule(EUSCI_A0_BASE, &uartConfig_9600);

    /* Enable UART module */
    UART_enableModule(EUSCI_A0_BASE);
}

void printDebug(char* message) {
		uint16_t i = 0;
		while (message[i] != '\0') {
			while(!(UCA0IFG&UCTXIFG));
			UART_transmitData(EUSCI_A0_BASE, reverse(message[i]));
			i++;
		}
}
