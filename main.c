/* DriverLib Includes */
#include "debug.h"
#include "driverlib.h"
#include "stepper.h"
#include "time_and_down.h"
#include "seven_segment.h"
#include "helpers.h"
#include "cmd.h"

/* Standard Includes */
#include <stdint.h>
#include <stdio.h>

#include <stdbool.h>
#include <string.h>

volatile int8_t player1_move = 0;
volatile int8_t player2_move = 0;
volatile bool player1_locked = false;
volatile bool player2_locked = false;
volatile bool player1_offense = true;
volatile bool ttt_game_completed = false;
volatile bool ttt_player1_won = false;

#define TIC_TAC_TOE -1
#define ROCK_PAPER_SCISSORS -2
#define SIMON_SAYS -3
#define TRIVIA -4


typedef struct {
    int8_t player1;
    int8_t player2;
    double probability;
    int8_t outcome1; // negative indicates a minigame
    int8_t outcome2;
} play_combo_t;

play_combo_t play_combos[] = {
    {1, 1, 1.00, ROCK_PAPER_SCISSORS, 5},
    {1, 2, 0.80, 5, 0},
    {1, 3, 0.75, 10, 0},
    {1, 4, 0.90, 5, 0},
    {2, 1, 0.80, 5, 0},
    {2, 2, 1.00, TIC_TAC_TOE, 5},
    {2, 3, 0.90, 5, 0},
    {2, 4, 0.80, 10, 5},
    {3, 1, 0.85, 5, 0},
    {3, 2, 0.75, 5, 0},
    {3, 3, 1.00, TRIVIA, 5},
    {3, 4, 0.80, 15, 5},
    {4, 1, 0.75, 10, 0},
    {4, 2, 0.80, 15, 5},
    {4, 3, 0.80, 15, 5},
    {4, 4, 1.00, SIMON_SAYS, 5},
};

// ROCK PAPER SCISSORS

#define ROCK 1
#define PAPER 2
#define SCISSORS 4
typedef struct {
    int8_t player1;
    int8_t player2;
    bool player1_wins;
} rps_combo_t;

rps_combo_t rps_combos[] = {
    {ROCK, ROCK, false},
    {ROCK, PAPER, false},
    {ROCK, SCISSORS, true},
    {PAPER, ROCK, true},
    {PAPER, PAPER, false},
    {PAPER, SCISSORS, false},
    {SCISSORS, ROCK, false},
    {SCISSORS, PAPER, true},
    {SCISSORS, SCISSORS, false},
};

// TRIVIA

typedef struct {
    char question[100];
    char choice1[100];
    char choice2[100];
    char choice3[100];
    char choice4[100];
    int8_t answer;
} trivia_question_t;

// SIMON SAYS
#define BLUE 1
#define GREEN 2
#define ORANGE 3
#define RED 4

volatile int8_t curr_player_seq = 0;
volatile int8_t simon_says_happening = false;
volatile bool simon_says_player_won = false;
volatile int8_t simon_says_seq[10];

int main(void)
{
    volatile uint32_t ii;
    char message[500];
    int i;
    int8_t outcome_res;
    int8_t trivia_choice;
    
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

    // Receive interrupt from the Pi
    UART_enableInterrupt(EUSCI_A0_BASE, EUSCI_A_UART_RECEIVE_INTERRUPT);
    Interrupt_enableInterrupt(INT_EUSCIA0);

    // Receive interrupt from controller 1
    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P2,
            GPIO_PIN1 | GPIO_PIN2 | GPIO_PIN3, GPIO_PRIMARY_MODULE_FUNCTION);
    UART_initModule(EUSCI_A1_BASE, &uartConfig_9600);
    UART_enableModule(EUSCI_A1_BASE);
    UART_enableInterrupt(EUSCI_A1_BASE, EUSCI_A_UART_RECEIVE_INTERRUPT);
    Interrupt_enableInterrupt(INT_EUSCIA1);

    // Receive interrupt from controller 2
    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P3,
            GPIO_PIN1 | GPIO_PIN2 | GPIO_PIN3, GPIO_PRIMARY_MODULE_FUNCTION);
    UART_initModule(EUSCI_A2_BASE, &uartConfig_9600);
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

    // GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN0);
    // GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN0);
		
    // GPIO_setAsOutputPin(GPIO_PORT_P2, GPIO_PIN0);
    // GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN0);
		
    // GPIO_setAsOutputPin(GPIO_PORT_P2, GPIO_PIN1);
    // GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN1);
		
    // GPIO_setAsOutputPin(GPIO_PORT_P2, GPIO_PIN2);
    // GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN2);
	
	
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
    sendCmd("WELCOME");
    movefootball(-120);
    moveFootballToYardage(25);
    // TODO: remove
    startTime();


    // TODO: while game is on
    while (false) {
        sendCmd("SELECT_PLAY");
        startTime();

        player1_locked = false;
        player2_locked = false;

        /* Going to LPM3 */
        while (!player1_locked || !player2_locked)
        {
            PCM_gotoLPM0();
        }

        if (getDown() == 4) {
            // Player 1 Punts
            if (player1_offense && player1_move == 1) {
                moveFootballToYardage(getYardage() + 30);
                player1_offense = false;
                setDown(1);
                setDistance(10);
                continue;
            // Player 1 Field Goals
            } else if (player1_offense && player1_move == 4) {
                if (getYardage() > 70) {
                    sendCmd("OUTCOME -h \"Field Goal is Good!\"");
                    incrementPlayer1ScoreBy(3);
                    // Move football to 75 yardline for kickoff
                    moveFootballToYardage(75);
                } else if (getYardage() > 60 && getYardage() <= 70) {
                    if (r2() > 0.5) {
                        sendCmd("OUTCOME -h \"Field Goal is Good!\"");
                        incrementPlayer2ScoreBy(3);
                        // Move football to 75 yardline for kickoff
                        moveFootballToYardage(75);
                    } else {
                        sendCmd("OUTCOME -h \"Field Goal is No Good!\"");
                    }
                } else {
                    sendCmd("OUTCOME -h \"Field Goal is No Good!\"");
                }
                player1_offense = false;
                setDistance(10);
                setDown(1);
                continue;
            // Player 2 Punts
            } else if (!player1_offense && player2_move == 1) {
                moveFootballToYardage(getYardage() - 30);
                player1_offense = true;
                setDistance(10);
                setDown(1);
                continue;
            // Player 2 Field Goals
            } else if (!player1_offense && player2_move == 4) {
                if (getYardage() < 30) {
                    sendCmd("OUTCOME -h \"Field Goal is Good!\"");
                    incrementPlayer2ScoreBy(3);
                    // Move football to 25 yardline for kickoff
                    moveFootballToYardage(25);
                } else if(getYardage() < 40 && getYardage() >= 30) {
                    if (r2() > 0.5) {
                        sendCmd("OUTCOME -h \"Field Goal is Good!\"");
                        incrementPlayer2ScoreBy(3);
                        // Move football to 25 yardline for kickoff
                        moveFootballToYardage(25);
                    } else {
                        sendCmd("OUTCOME -h \"Field Goal is No Good!\"");
                    }
                } else {
                    sendCmd("OUTCOME -h \"Field Goal is No Good!\"");
                }
                player1_offense = true;
                setDistance(10);
                setDown(1);
                continue;
            }
        }
        
        for (i = 0; i < 16; i++) {
            if (play_combos[i].player1 == player1_move && play_combos[i].player2 == player2_move) {
                if (r2() < play_combos[i].probability) {
                    outcome_res = play_combos[i].outcome1;
                } else {
                    outcome_res = play_combos[i].outcome2;
                }
                break;
            }
        }

        if (outcome_res < 0) {
            pauseTime();
            switch (outcome_res) {
                case TIC_TAC_TOE:
                    ttt_game_completed = false;
                    while (!ttt_game_completed)
                    {
                        PCM_gotoLPM0();
                    }
                    if (player1_offense && ttt_player1_won) {
                        moveFootballToYardage(getYardage() + 10);

                    } else if (!player1_offense && !ttt_player1_won) {
                        moveFootballToYardage(getYardage() - 10);
                    }
                    break;
                case SIMON_SAYS:
                    for (i = 0; i < 10; i++) {
                        if (r2() < 0.25) {
                            simon_says_seq[i] = BLUE;
                        } else if (r2() < 0.5) {
                            simon_says_seq[i] = ORANGE;
                        } else if (r2() < 0.75) {
                            simon_says_seq[i] = GREEN;
                        } else {
                            simon_says_seq[i] = RED;
                        }
                    }

                    curr_player_seq = 0;
                    simon_says_happening = true;

                    while (simon_says_happening)
                    {
                        PCM_gotoLPM0();
                    }
                    // Offense played
                    if (player1_offense && simon_says_player_won) {
                        moveFootballToYardage(getYardage() + 20);
                    } else if (!player1_offense && simon_says_player_won) {
                        moveFootballToYardage(getYardage() - 20);
                    }

                    break;
                case ROCK_PAPER_SCISSORS:
                    player1_locked = false;
                    player2_locked = false;

                    /* Going to LPM3 */
                    while (!player1_locked || !player2_locked)
                    {
                        PCM_gotoLPM0();
                    }

                    for (i = 0; i < 9; i++) {
                        if (rps_combos[i].player1 == player1_move && rps_combos[i].player2 == player2_move) {
                            if (player1_offense && rps_combos[i].player1_wins) {
                                moveFootballToYardage(getYardage() + 10);                                
                            }
                        // Player 2 uses Player 1's results in the array
                        } else if (rps_combos[i].player2 == player1_move && rps_combos[i].player1 == player2_move) {
                            if (!player1_offense && rps_combos[i].player1_wins) {
                                moveFootballToYardage(getYardage() - 10);                                
                            }
                        }
                    }

                    // if (player1_move ==)
                    break;
                case TRIVIA:
                    // TODO: Send question
                    // sprintf(message, "TRIVIA -q \"%s\" \"%s\" \"%s\" \"%s\" \"%s\"", question, choice1, choice2, choice3, choice4);
                    sendCmd("TRIVIA");
                    // Defense Answers
                    if (player1_offense) {
                        player2_locked = false;
                        while (!player2_locked)
                        {
                            PCM_gotoLPM0();
                        }
                        trivia_choice = player2_move;
                    } else {
                        player1_locked = false;      
                        while (!player1_locked)
                        {
                            PCM_gotoLPM0();
                        }
                        trivia_choice = player1_move;
                    }
                    // TODO: Send question with the answer
                    // sprintf(message, "TRIVIA -q \"%s\" \"%s\" \"%s\" \"%s\ %d" \"%s\"", question, choice1, choice2, choice3, choice4, answer);

                    // TODO: define answer from array
                    // if (trivia_choice != answer) {
                    //     if (player1_offense) {
                    //         moveFootballToYardage(getYardage() + 10);        
                    //     } else {
                    //         moveFootballToYardage(getYardage() - 10);        
                    //     }
                    // }
                    break;
                default:
                    // printDebug("INVALID NEGATIVE OUTCOME: %d\n", outcome_res);
                    break;
            }
        } else {
            if (player1_offense) {
                moveFootballToYardage(getYardage() + outcome_res);
                if (getYardage() >= 100) {
                    sendCmd("OUTCOME -h \"TOUCHDOWN\"");
                    incrementPlayer1ScoreBy(7);
                    moveFootballToYardage(75);
                    setDown(1);
                    setDistance(10);
                }
            } else {
                moveFootballToYardage(getYardage() - outcome_res);
                if (getYardage <= 0) {
                    sendCmd("OUTCOME -h \"TOUCHDOWN\"");
                    incrementPlayer2ScoreBy(7);
                    moveFootballToYardage(25);
                    setDown(1);
                    setDistance(10);
                }
            }
        }

        // TODO: update down

        // Still set to fourth down after play
        if (getDown() == 4) {
            player1_offense = !player1_offense;
            setDown(1);
            setDistance(10);
        }
    }

        while(1) {
            PCM_gotoLPM0();
        }

}


// Pi's receive interrupt handler
void EUSCIA0_IRQHandler(void)
{
    uint32_t status = UART_getEnabledInterruptStatus(EUSCI_A0_BASE);
		
    UART_clearInterruptFlag(EUSCI_A0_BASE, status);

    if(status & EUSCI_A_UART_RECEIVE_INTERRUPT_FLAG)
    {
        if (!ttt_game_completed) {
            int8_t data = reverse(UART_receiveData(EUSCI_A0_BASE));
            ttt_player1_won = data == 0;
            ttt_game_completed = true;
        }
    }
}

// Controller 1's receive interrupt handler
void EUSCIA1_IRQHandler(void)
{
    char message[100];
    uint32_t status = UART_getEnabledInterruptStatus(EUSCI_A1_BASE);
		
    UART_clearInterruptFlag(EUSCI_A1_BASE, status);

    if(status & EUSCI_A_UART_RECEIVE_INTERRUPT_FLAG)
    {
        int8_t data = reverse(UART_receiveData(EUSCI_A1_BASE));

        if (!player1_locked || (player1_offense && simon_says_happening)) {
            player1_move = data;
            player1_locked = true;
            
            if (simon_says_happening) {
                if (simon_says_seq[curr_player_seq] != player1_move) {
                    simon_says_player_won = false;
                    simon_says_happening = false;
                } else {
                    curr_player_seq++; 
                }
            }
            if (simon_says_happening && curr_player_seq == 10) {
                simon_says_player_won = true;
                simon_says_happening = false;
            }
        }
        // sprintf(message, "detection_status=%d\n", data);
        // printDebug(message);
    }
}

// Controller 2's receive interrupt handler
void EUSCIA2_IRQHandler(void)
{
        char message[100];
		uint32_t status = UART_getEnabledInterruptStatus(EUSCI_A2_BASE);
		
    UART_clearInterruptFlag(EUSCI_A2_BASE, status);

    if(status & EUSCI_A_UART_RECEIVE_INTERRUPT_FLAG)
    {
        int8_t data = reverse(UART_receiveData(EUSCI_A2_BASE));

        if (!player2_locked || (!player1_offense && simon_says_happening)) {
            player2_move = data;
            player2_locked = true;
            
            if (simon_says_happening) {
                if (simon_says_seq[curr_player_seq] != player1_move) {
                    simon_says_player_won = false;
                    simon_says_happening = false;
                } else {
                    curr_player_seq++; 
                }
            }
            if (simon_says_happening && curr_player_seq == 10) {
                simon_says_player_won = true;
                simon_says_happening = false;
            }
        }
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
