#ifndef __TIME_AND_DOWN_H__
#define __TIME_AND_DOWN_H__

#include <stdint.h>

uint32_t getPlayer1Score(void);
void setPlayer1Score(uint32_t new_score);
void incrementPlayer1ScoreBy(uint32_t score);
uint32_t getPlayer2Score(void);
void setPlayer2Score(uint32_t new_score);
void incrementPlayer2ScoreBy(uint32_t score);

void initTimeAndDown(void);

uint8_t getQuarter(void);
void incrementQuarter(void);
void resetQuarter(void);
void setQuarter(uint8_t newQuarter);

void resetTime(void);
void startTime(void);
void pauseTime(void);

uint8_t getDown(void);
void incrementDown(void);
void resetDown(void);
void setDown(uint8_t newDown);

uint8_t getDistance(void);
void setDistance(uint8_t new_distance);
void resetDistance(void);

#endif
