/* DriverLib Includes */
#include "driverlib.h"
#include "helpers.h"

/* Standard Includes */
#include <stdint.h>

#include <stdbool.h>
#include <string.h>
#include "debug.h"



void printDebug(char* message) {
		uint16_t i = 0;
		while (message[i] != '\0') {
			while(!(UCA0IFG&UCTXIFG));
			UART_transmitData(EUSCI_A0_BASE, reverse(message[i]));
			i++;
		}
}
