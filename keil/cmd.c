#include <stdarg.h>
#include "driverlib.h"
#include "cmd.h"
#include "helpers.h"

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
