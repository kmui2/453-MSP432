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

#define NUM_PLAY_COMBOS 16
typedef struct
{
    int8_t player1;
    int8_t player2;
    float probability;
    int8_t outcome1; // negative indicates a minigame
    int8_t outcome2;
} play_combo_t;

play_combo_t play_combos[] = {
    {1, 1, 1.00, ROCK_PAPER_SCISSORS, 5},
    {1, 2, 0.70, 5, 0},
    {1, 3, 0.65, 10, 0},
    {1, 4, 0.80, 5, 0},
    {2, 1, 0.70, 5, 0},
    {2, 2, 1.00, TIC_TAC_TOE, 5},
    {2, 3, 0.80, 5, 0},
    {2, 4, 0.70, 10, 5},
    {3, 1, 0.75, 5, 0},
    {3, 2, 0.65, 5, 0},
    {3, 3, 1.00, TRIVIA, 5},
    {3, 4, 0.70, 15, 5},
    {4, 1, 0.65, 10, 0},
    {4, 2, 0.70, 15, 5},
    //{4, 2, 1.00, 50, 5},
    {4, 3, 0.70, 15, 5},
    {4, 4, 1.00, SIMON_SAYS, 5},
    //{4, 4, 1.00, TRIVIA, 5},
};

// ROCK PAPER SCISSORS

#define ROCK 1
#define PAPER 2
#define SCISSORS 4
typedef struct
{
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

typedef struct
{
    char question[100];
    char choice1[50];
    char choice2[50];
    char choice3[50];
    char choice4[50];
    int8_t answer;
} trivia_question_t;

#define NUM_TRIVIA_QUESTIONS 10

uint8_t curr_trivia_question = 0;

trivia_question_t trivia_questions[] = {
    {"What is the name of the trophy won in a UW vs UMN football game?", "Heartland Trophy", "Freedom Trophy", "Paul Bunyan's Axe", "Little Brown Jug", 2},
    {"What is the name of UW's football stadium?", "Lambeu Field", "Camp Randall", "Miller Park", "Kohl Center", 1},
    {"What is the name of UMN's football stadium?", "TCF Bank Stadium", "US Bank Stadium", "Metrodome", "Target Field", 0},
    {"Who is UW's current coach (in May of 2019)?", "Barry Alvarez", "Brett Bielama", "Bo Ryan", "Paul Chryst", 3},
    {"Who is UMN's current coach (in May of 2019)?", "Rick Pitino", "PJ Fleck", "Richard Pitino", "Jerry Kill", 1},
    {"How many meetings were there between UW and UWM as of the end of 2018?", "128", "172", "87", "101", 0},
    {"Who has more wins in the UW-UMN football series?", "Golden Gophers", "Badgers", "Tied", "Hawkeyes", 2},
    {"What is the name of the trophy in UW-UMN football game before the axe?", "Heartland Trophy", "River Jug", "Slab of Bacon", "Rodent of Trophy", 2},
    {"Who holds the longest win streak in the UW-UMN football series?", "Spartans", "Buckeyes", "Golden Gophers", "Badgers", 3},
    {"What is the only year since 1890 UW and UMN did not play each other in football?", "1942", "1906", "1963", "1929", 1},
};

// SIMON SAYS
#define BLUE 1
#define GREEN 2
#define ORANGE 3
#define RED 4

volatile int8_t curr_player_seq = 0;
volatile int8_t simon_says_happening = false;
volatile bool simon_says_player_won = false;
volatile int8_t simon_says_seq[10];

/* Application Defines  */
#define TIMER_PERIOD 0x2DC6

int main(void)
{
    volatile uint32_t ii;
    char message[500];
    int i;
    int8_t outcome_res;
    int8_t trivia_choice;
    bool scoredTouchdown = false;
    float res;

    const eUSCI_UART_Config uartConfig_9600 =
        {
            EUSCI_A_UART_CLOCKSOURCE_SMCLK,               // SMCLK Clock Source
            78,                                           // BRDIV = 13
            2,                                            // UCxBRF = 0
            0,                                            // UCxBRS = 37
            EUSCI_A_UART_NO_PARITY,                       // No Parity
            EUSCI_A_UART_MSB_FIRST,                       // MSB First
            EUSCI_A_UART_ONE_STOP_BIT,                    // One stop bit
            EUSCI_A_UART_MODE,                            // UART mode
            EUSCI_A_UART_OVERSAMPLING_BAUDRATE_GENERATION // Oversampling
        };

    const Timer_A_ContinuousModeConfig continuousModeConfig =
        {
            TIMER_A_CLOCKSOURCE_ACLK,      // ACLK Clock Source
            TIMER_A_CLOCKSOURCE_DIVIDER_1, // ACLK/1 = 32.768khz
            TIMER_A_TAIE_INTERRUPT_ENABLE, // Enable Overflow ISR
            TIMER_A_DO_CLEAR               // Clear Counter
        };

    /* Timer_A UpMode Configuration Parameter */
    const Timer_A_UpModeConfig upConfig =
        {
            TIMER_A_CLOCKSOURCE_SMCLK,          // SMCLK Clock Source
            TIMER_A_CLOCKSOURCE_DIVIDER_64,     // SMCLK/1 = 3MHz
            TIMER_PERIOD,                       // 5000 tick period
            TIMER_A_TAIE_INTERRUPT_DISABLE,     // Disable Timer interrupt
            TIMER_A_CCIE_CCR0_INTERRUPT_ENABLE, // Enable CCR0 interrupt
            TIMER_A_DO_CLEAR                    // Clear value
        };

    /* Halting the Watchdog */
    WDT_A_holdTimer();

    /* Setting DCO to 12MHz */
    CS_setDCOCenteredFrequency(CS_DCO_FREQUENCY_12);

    /////////////////
    // Timers
    /////////////////
    /* Configuring Timer_A1 for Up Mode */
    Timer_A_configureUpMode(TIMER_A1_BASE, &upConfig);

    /////////////////
    // UART
    /////////////////

    // Receive interrupt from the Pi
    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P1,
                                               GPIO_PIN1 | GPIO_PIN2 | GPIO_PIN3, GPIO_PRIMARY_MODULE_FUNCTION);
    UART_initModule(EUSCI_A0_BASE, &uartConfig_9600);
    UART_enableModule(EUSCI_A0_BASE);
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
    // TODO: add me back
    //movefootball(-120);
    moveFootballToYardage(25);
    delayTimer(3);

    // while game is on
    while (!game_over)
    // while (false)
    {
        if (halftime)
        {
            // Show halftime screen
            sendCmd("OUTCOME Halftime");
            player1_offense = false;
            resetTimeAndDown(player1_offense);
            moveFootballToYardage(75);
            halftime = false;
            delayTimer(3);
        }
        scoredTouchdown = false;
        sendCmd("SELECT_PLAY");
        startTime();

        player1_locked = false;
        player2_locked = false;

        /* Going to LPM3 */
        while (!player1_locked || !player2_locked)
        {
            PCM_gotoLPM0();
        }

        if (getDown() == 4)
        {
            // Player 1 Punts
            if (player1_offense && player1_move == 1)
            {
                sendCmd("OUTCOME \"Player 1 Punts\"");
                if (getYardage() > 70)
                {
                    moveFootballToYardage(100);
                }
                else
                {
                    moveFootballForwardBy(+30);
                }
                player1_offense = false;
                resetTimeAndDown(player1_offense);
                delayTimer(3);
                continue;
            }
            // Player 1 Field Goals
            else if (player1_offense && player1_move == 4)
            {
                if (getYardage() > 70)
                {
                    sendCmd("OUTCOME \"Player 1's Field Goal is Good!\"");
                    incrementPlayer1ScoreBy(3);
                    // Move football to 75 yardline for kickoff
                    moveFootballToYardage(75);
                    delayTimer(3);
                }
                else if (getYardage() > 60 && getYardage() <= 70)
                {
                    if (r2() > 0.5)
                    {
                        sendCmd("OUTCOME \"Player 1's Field Goal is Good!\"");
                        incrementPlayer2ScoreBy(3);
                        // Move football to 75 yardline for kickoff
                        moveFootballToYardage(75);
                        delayTimer(3);
                    }
                    else
                    {
                        sendCmd("OUTCOME \"Player 1's Field Goal is No Good!\"");
                        delayTimer(3);
                    }
                }
                else
                {
                    sendCmd("OUTCOME \"Player 1's Field Goal is No Good!\"");
                    delayTimer(3);
                }
                player1_offense = false;
                resetTimeAndDown(player1_offense);
                continue;
            }
            // Player 2 Punts
            else if (!player1_offense && player2_move == 1)
            {
                sendCmd("OUTCOME \"Player 2 Punts\"");
                if (getYardage() < 30)
                {
                    moveFootballToYardage(0);
                }
                else
                {
                    moveFootballForwardBy(-30);
                }
                player1_offense = true;
                resetTimeAndDown(player1_offense);
                delayTimer(3);
                continue;
            }
            // Player 2 Field Goals
            else if (!player1_offense && player2_move == 4)
            {
                if (getYardage() < 30)
                {
                    sendCmd("OUTCOME \"Player 2's Field Goal is Good!\"");
                    incrementPlayer2ScoreBy(3);
                    // Move football to 25 yardline for kickoff
                    moveFootballToYardage(25);
                    delayTimer(3);
                }
                else if (getYardage() < 40 && getYardage() >= 30)
                {
                    if (r2() > 0.5)
                    {
                        sendCmd("OUTCOME \"Player 2's Field Goal is Good!\"");
                        incrementPlayer2ScoreBy(3);
                        // Move football to 25 yardline for kickoff
                        moveFootballToYardage(25);
                        delayTimer(3);
                    }
                    else
                    {
                        sendCmd("OUTCOME \"Player 2's Field Goal is No Good!\"");
                        delayTimer(3);
                    }
                }
                else
                {
                    sendCmd("OUTCOME \"Player 2's Field Goal is No Good!\"");
                    delayTimer(3);
                }
                player1_offense = true;
                resetTimeAndDown(player1_offense);
                continue;
            }
        }

        // Normal plays
        for (i = 0; i < NUM_PLAY_COMBOS; i++)
        {
            if (player1_offense && (play_combos[i].player1 == player1_move && play_combos[i].player2 == player2_move))
            {

                res = r2();
                if (res < play_combos[i].probability)
                {
                    outcome_res = play_combos[i].outcome1;
                }
                else
                {
                    outcome_res = play_combos[i].outcome2;
                }
                break;
            }
            else if (!player1_offense && (play_combos[i].player1 == player2_move && play_combos[i].player2 == player1_move))
            {
                res = r2();
                if (res < play_combos[i].probability)
                {
                    outcome_res = play_combos[i].outcome1;
                }
                else
                {
                    outcome_res = play_combos[i].outcome2;
                }
                break;
            }
        }

        if (outcome_res >= 0)
        {
            if (player1_offense)
            {
                char msg[20];
                sprintf(msg, "OUTCOME \"Player 1 Moves %i Yards\"", outcome_res);
                sendCmd(msg);
                scoredTouchdown = moveFootballForwardBy(outcome_res);
                decrementDistanceBy(outcome_res);
                delayTimer(3);
            }
            else
            {
                char msg[20];
                sprintf(msg, "OUTCOME \"Player 2 Moves %i Yards\"", outcome_res);
                sendCmd(msg);
                scoredTouchdown = moveFootballForwardBy(-outcome_res);
                decrementDistanceBy(outcome_res);
                delayTimer(3);
            }
        }
        // Outcome is a minigame (defined by a negative number)
        else
        {
            switch (outcome_res)
            {
            case TIC_TAC_TOE:
                // 1. Change screen to TIC TAC TOE
                // 2. Wait for game to complete
                // 3. Move the ball by the result (if there is a tie, nothing hapens)
                pauseTime();
                sendCmd("TIC_TAC_TOE");
                ttt_game_completed = false;
                while (!ttt_game_completed)
                {
                    PCM_gotoLPM0();
                }

                // Let the results screen display for 3 seconds
                delayTimer(3);

                startTime();
                if (player1_offense && ttt_player1_won)
                {
                    sendCmd("OUTCOME \"Player 1 moves 10 yards\"");
                    scoredTouchdown = moveFootballForwardBy(10);
                    decrementDistanceBy(10);
                    delayTimer(3);
                }
                else if (!player1_offense && !ttt_player1_won)
                {
                    sendCmd("OUTCOME \"Player 2 moves 10 yards\"");
                    scoredTouchdown = moveFootballForwardBy(-10);
                    decrementDistanceBy(10);
                    delayTimer(3);
                }
                break;
            case SIMON_SAYS:
            {
                uint8_t last_color = 0xFF;
                // 1. Create simon says color sequence
                // 2. Start simon says get ready
                // 3. Start color sequence
                // 4. Start simon says game
                // 5. Poll until simon says is completed (interrupt handler will determine when it's over and whether the offense won)
                // 6. Show results on Outcome screen
                // 7. Move the ball by the result
                pauseTime();
                for (i = 0; i < 10; i++)
                {
                    res = r2();
                    if (res < 0.25)
                    {
                        simon_says_seq[i] = BLUE;
                    }
                    else if (res < 0.5)
                    {
                        simon_says_seq[i] = ORANGE;
                    }
                    else if (res < 0.75)
                    {
                        simon_says_seq[i] = GREEN;
                    }
                    else
                    {
                        simon_says_seq[i] = RED;
                    }

                    if (simon_says_seq[i] == last_color)
                    {
                        i--;
                    }
                    else
                    {
                        last_color = simon_says_seq[i];
                    }
                }

                sendCmd("SIMON_SAYS");
                delayTimer(5);

                for (i = 0; i < 10; i++)
                {
                    switch (simon_says_seq[i])
                    {
                    case BLUE:
                        sendCmd("SIMON_SAYS blue");
                        break;
                    case ORANGE:
                        sendCmd("SIMON_SAYS orange");
                        break;
                    case GREEN:
                        sendCmd("SIMON_SAYS green");
                        break;
                    case RED:
                        sendCmd("SIMON_SAYS red");
                        break;
                    }
                    delayTimer(3);
                }
                sendCmd("SIMON_SAYS white");

                curr_player_seq = 0;
                simon_says_happening = true;

                while (simon_says_happening)
                {
                    PCM_gotoLPM0();
                }

                startTime();
                // Offense played
                if (player1_offense && simon_says_player_won)
                {
                    scoredTouchdown = moveFootballForwardBy(20);
                    sendCmd("OUTCOME \"Player 1 moves 20 yards\"");
                    decrementDistanceBy(20);
                    delayTimer(3);
                }
                else if (!player1_offense && simon_says_player_won)
                {
                    scoredTouchdown = moveFootballForwardBy(-20);
                    sendCmd("OUTCOME \"Player 2 moves 20 yards\"");
                    decrementDistanceBy(20);
                    delayTimer(3);
                }
                else
                {
                    sendCmd("OUTCOME LOST");
                    delayTimer(3);
                }

                break;
            }
            case ROCK_PAPER_SCISSORS:
                // 1. Show Rock Paper and Scissors game placeholder
                // 2. Poll until both players choose their move
                // 3. Show results on Outcome screen
                // 4. Move the ball by the result
                pauseTime();
                sendCmd("ROCK_PAPER_SCISSORS");
                delayTimer(3);
                player1_locked = false;
                player2_locked = false;

                /* Going to LPM3 */
                while (!player1_locked || !player2_locked)
                {
                    PCM_gotoLPM0();
                }
                startTime();

                // Loop not picking an outcome
                for (i = 0; i < 9; i++)
                {
                    if (rps_combos[i].player1 == player1_move && rps_combos[i].player2 == player2_move)
                    {
                        if (player1_offense && rps_combos[i].player1_wins)
                        {
                            sendCmd("OUTCOME \"Player 1 moves 10 yards\"");
                            scoredTouchdown = moveFootballForwardBy(10);
                            decrementDistanceBy(10);
                            delayTimer(3);
                        }
                        break;
                        // Player 2 uses Player 1's results in the array
                    }
                    else if (rps_combos[i].player2 == player1_move && rps_combos[i].player1 == player2_move)
                    {
                        if (!player1_offense && rps_combos[i].player1_wins)
                        {
                            sendCmd("OUTCOME \"Player 2 moves 10 yards\"");
                            scoredTouchdown = moveFootballForwardBy(-10);
                            decrementDistanceBy(10);
                            delayTimer(3);
                        }
                        break;
                    }
                }
                break;
            case TRIVIA:
                // 1. Show trivia screen with question
                // 2. Poll for both responses, Only defense answers question
                //if defense correct nothing, if incorrect move ball
                // 3. Show trivia answer
                // 4. Show results on Outcome screen
                // 4. Move the ball by the result
                pauseTime();

                sprintf(message, "TRIVIA \"%s\" \"%s\" \"%s\" \"%s\" \"%s\"",
                        trivia_questions[curr_trivia_question].question,
                        trivia_questions[curr_trivia_question].choice1,
                        trivia_questions[curr_trivia_question].choice2,
                        trivia_questions[curr_trivia_question].choice3,
                        trivia_questions[curr_trivia_question].choice4);
                sendCmd(message);
                // Defense Answers
                if (player1_offense)
                {
                    player2_locked = false;
                    while (!player2_locked)
                    {
                        PCM_gotoLPM0();
                    }
                    trivia_choice = player2_move;
                }
                else
                {
                    player1_locked = false;
                    while (!player1_locked)
                    {
                        PCM_gotoLPM0();
                    }
                    trivia_choice = player1_move;
                }

                // Send question with the answer
                sprintf(message, "TRIVIA \"%s\" \"%s\" \"%s\" \"%s\" \"%s\" \"%d\"",
                        trivia_questions[curr_trivia_question].question,
                        trivia_questions[curr_trivia_question].choice1,
                        trivia_questions[curr_trivia_question].choice2,
                        trivia_questions[curr_trivia_question].choice3,
                        trivia_questions[curr_trivia_question].choice4,
                        trivia_questions[curr_trivia_question].answer);
                sendCmd(message);
                delayTimer(3);

                startTime();
                if (trivia_choice == trivia_questions[curr_trivia_question].answer)
                {
                    if (player1_offense)
                    {

                        sendCmd("OUTCOME \"Player 1 moves 10 yards\"");
                        scoredTouchdown = moveFootballForwardBy(10);
                        decrementDistanceBy(10);
                        delayTimer(5);
                    }
                    else
                    {
                        sendCmd("OUTCOME \"Player 2 moves 10 yards\"");
                        scoredTouchdown = moveFootballForwardBy(-10);
                        decrementDistanceBy(10);
                        delayTimer(5);
                    }
                }
                else
                {
                    sendCmd("OUTCOME \"Wrong answer.\"");
                    delayTimer(3);
                }

                curr_trivia_question++;
                break;
            default:
                // printDebug("INVALID NEGATIVE OUTCOME: %d\n", outcome_res);
                break;
            }
        }

        if (scoredTouchdown)
        {
            if (player1_offense)
            {
                sendCmd("OUTCOME \"Player 1 scores a TOUCHDOWN!\"");
                incrementPlayer1ScoreBy(7);
                moveFootballToYardage(75);
            }
            else
            {
                sendCmd("OUTCOME \"Player 2 scores a TOUCHDOWN!\"");
                incrementPlayer2ScoreBy(7);
                moveFootballToYardage(25);
            }
            player1_offense = !player1_offense;
            resetTimeAndDown(player1_offense);
            delayTimer(3);
        }
        else
        {
            // update down and distance
            if (getDistance() <= 0)
            {
                resetTimeAndDown(player1_offense);
            }
            else
            {
                incrementDown();
            }
        }

        // Still set to fourth down after play
        if (getDown() > 4)
        {
            if (player1_offense)
            {
                sendCmd("OUTCOME \"Turnover to Player 2\"");
            }
            else
            {
                sendCmd("OUTCOME \"Turnover to Player 1\"");
            }
            player1_offense = !player1_offense;
            resetTimeAndDown(player1_offense);
            delayTimer(3);
        }
    }

    // Game over
    if (getPlayer1Score() > getPlayer2Score())
    {
        sendCmd("OUTCOME \"Player 1 won!\"");
    }
    else if (getPlayer1Score() < getPlayer2Score())
    {
        sendCmd("OUTCOME \"Player 2 won!\"");
    }
    else
    {
        sendCmd("OUTCOME \"It's a tie!\"");
    }
    while (1)
    {
        PCM_gotoLPM0();
    }
}

// Pi's receive interrupt handler
void EUSCIA0_IRQHandler(void)
{
    uint32_t status = UART_getEnabledInterruptStatus(EUSCI_A0_BASE);

    UART_clearInterruptFlag(EUSCI_A0_BASE, status);

    if (status & EUSCI_A_UART_RECEIVE_INTERRUPT_FLAG)
    {
        if (!ttt_game_completed)
        {
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

    if (status & EUSCI_A_UART_RECEIVE_INTERRUPT_FLAG)
    {
        int8_t data = reverse(UART_receiveData(EUSCI_A1_BASE));

        if ((!player1_locked || (player1_offense && simon_says_happening)) && (data == 1 || data == 2 || data == 4 || data == 8))
        {
            player1_move = data;
            player1_locked = true;

            if (simon_says_happening)
            {
                if (simon_says_seq[curr_player_seq] != player1_move)
                {
                    simon_says_player_won = false;
                    simon_says_happening = false;
                }
                else
                {
                    curr_player_seq++;
                }
            }
            if (simon_says_happening && curr_player_seq == 10)
            {
                simon_says_player_won = true;
                simon_says_happening = false;
            }
            //sprintf(message, "Controller 1: %d\n", player1_move);
            //printDebug(message);
        }
    }
}

// Controller 2's receive interrupt handler
void EUSCIA2_IRQHandler(void)
{
    char message[100];
    uint32_t status = UART_getEnabledInterruptStatus(EUSCI_A2_BASE);

    UART_clearInterruptFlag(EUSCI_A2_BASE, status);

    if (status & EUSCI_A_UART_RECEIVE_INTERRUPT_FLAG)
    {
        int8_t data = reverse(UART_receiveData(EUSCI_A2_BASE));

        if ((!player2_locked || (!player1_offense && simon_says_happening)) && (data == 1 || data == 2 || data == 4 || data == 8))
        {
            player2_move = data;
            player2_locked = true;

            if (simon_says_happening)
            {
                if (simon_says_seq[curr_player_seq] != player1_move)
                {
                    simon_says_player_won = false;
                    simon_says_happening = false;
                }
                else
                {
                    curr_player_seq++;
                }
            }
            if (simon_says_happening && curr_player_seq == 10)
            {
                simon_says_player_won = true;
                simon_says_happening = false;
            }
            //sprintf(message, "Controller 2: %d\n", player2_move);
            //printDebug(message);
        }
    }
}

/* GPIO ISR */
void PORT1_IRQHandler(void)
{
    uint32_t status;

    status = GPIO_getEnabledInterruptStatus(GPIO_PORT_P1);
    GPIO_clearInterruptFlag(GPIO_PORT_P1, status);

    /* Toggling the output on the LED */
    if (status & GPIO_PIN1)
    {
        GPIO_toggleOutputOnPin(GPIO_PORT_P1, GPIO_PIN0);
    }

    if (status & GPIO_PIN4)
    {
        GPIO_toggleOutputOnPin(GPIO_PORT_P2, GPIO_PIN0);
    }
}
