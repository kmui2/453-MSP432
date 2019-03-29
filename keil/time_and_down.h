#ifndef __TIME_AND_DOWN_H__
#define __TIME_AND_DOWN_H__

#include <stdint.h>

void initTimeAndDown(void);

uint8_t getQuarter(void);
void incrementQuarter(void);
void resetQuarter(void);

void resetTime(void);
void startTime(void);
void pauseTime(void);

uint8_t getDown(void);
void incrementDown(void);
void resetDown(void);

uint8_t getDistance(void);
void setDistance(uint8_t new_distance);
void resetDistance(void);

#endif
