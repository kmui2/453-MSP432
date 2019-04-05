#include <stdarg.h>
#include "driverlib.h"
#include "cmd.h"
#include "helpers.h"

void setScreen(enum Screen screen) {
	char message[100];
	sprintf(message, "%s", (char*) screen);
	sendCmd(message);
}
