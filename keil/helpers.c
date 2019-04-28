#include "driverlib.h"
#include "helpers.h"
#include "stdlib.h"

unsigned char reverse(unsigned char b) {
   b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
   b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
   b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
   return b;
}

double r2()
{
    return (double)rand() / (double)RAND_MAX ;
}
