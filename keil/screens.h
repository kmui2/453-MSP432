#ifndef __SCREENS_H__
#define __SCREENS_H__

#include "driverlib.h"


enum SCREEN {
	MAIN = "MAIN",
	ROCK_PAPER_SCISSORS = "ROCK_PAPER_SCISSORS",
	SIMON_SAYS = "SIMON_SAYS",
	TIC_TAC_TOE = "TIC_TAC_TOE",
	TRIVIA = "TRIVIA",
	TIME_AND_DOWN = "TIME_AND_DOWN",
	WELCOME = "WELCOME",
	SELECT_PLAY = "SELECT_PLAY",
	OUTCOME = "OUTCOME",
	END_Q = "END_Q",
};

void setScreen(enum Screen screen);

#endif
