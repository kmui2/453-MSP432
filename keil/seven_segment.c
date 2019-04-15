#include "driverlib.h"
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#define SEVEN_SEGMENT_PORT GPIO_PORT_P4

#define D_PIN GPIO_PIN0
#define C_PIN GPIO_PIN1
#define B_PIN GPIO_PIN2
#define A_PIN GPIO_PIN3

 #define DEC_0_HIGH_PINS 0
#define DEC_0_LOW_PINS D_PIN | C_PIN | B_PIN | A_PIN 

 #define DEC_1_HIGH_PINS A_PIN
#define DEC_1_LOW_PINS D_PIN | C_PIN | B_PIN

 #define DEC_2_HIGH_PINS B_PIN
#define DEC_2_LOW_PINS D_PIN | C_PIN | A_PIN

 #define DEC_3_HIGH_PINS B_PIN | A_PIN
#define DEC_3_LOW_PINS D_PIN | C_PIN

 #define DEC_4_HIGH_PINS C_PIN
#define DEC_4_LOW_PINS D_PIN | B_PIN | A_PIN

 #define DEC_5_HIGH_PINS C_PIN | A_PIN
#define DEC_5_LOW_PINS D_PIN | B_PIN

 #define DEC_6_HIGH_PINS C_PIN | B_PIN
#define DEC_6_LOW_PINS D_PIN | A_PIN

 #define DEC_7_HIGH_PINS C_PIN | B_PIN | A_PIN
#define DEC_7_LOW_PINS D_PIN

 #define DEC_8_HIGH_PINS D_PIN
#define DEC_8_LOW_PINS C_PIN | B_PIN | A_PIN

 #define DEC_9_HIGH_PINS D_PIN | A_PIN
#define DEC_9_LOW_PINS C_PIN | B_PIN

 const uint_fast8_t decHighPins[] = {
  DEC_0_HIGH_PINS,
  DEC_1_HIGH_PINS,
  DEC_2_HIGH_PINS,
  DEC_3_HIGH_PINS,
  DEC_4_HIGH_PINS,
  DEC_5_HIGH_PINS,
  DEC_6_HIGH_PINS,
  DEC_7_HIGH_PINS,
  DEC_8_HIGH_PINS,
  DEC_9_HIGH_PINS,
};

 const uint_fast8_t decLowPins[] = {
  DEC_0_LOW_PINS,
  DEC_1_LOW_PINS,
  DEC_2_LOW_PINS,
  DEC_3_LOW_PINS,
  DEC_4_LOW_PINS,
  DEC_5_LOW_PINS,
  DEC_6_LOW_PINS,
  DEC_7_LOW_PINS,
  DEC_8_LOW_PINS,
  DEC_9_LOW_PINS,
};
 
void initSevenSegment() {
	GPIO_setAsOutputPin(SEVEN_SEGMENT_PORT, D_PIN | C_PIN | B_PIN | A_PIN);
	GPIO_setOutputLowOnPin(SEVEN_SEGMENT_PORT, D_PIN | C_PIN | B_PIN | A_PIN);	
}

void setSevenSegment(int dec) {
    GPIO_setOutputHighOnPin(SEVEN_SEGMENT_PORT, decHighPins[dec]);
    GPIO_setOutputLowOnPin(SEVEN_SEGMENT_PORT, decLowPins[dec]);
}

