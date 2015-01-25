#ifndef _SYSTEM_H_
#define _SYSTEM_H_

#include <stm32f4xx.h>

#define RAD_TO_DEG 57.295779513082320876798154814105

#define abs(x) ((x)>0?(x):-(x))
#define constrain(amt,low,high) ((amt)<(low)?(low):((amt)>(high)?(high):(amt)))
#define degrees(rad) ((rad)*RAD_TO_DEG)

#define digitalHi(p, i)     { p->BSRR = i; }
#define digitalLo(p, i)     { p->BRR = i; }
#define digitalToggle(p, i) { p->ODR ^= i; }
#define digitalIn(p, i)     (p->IDR & i)

void systemInit(void);
uint32_t getSystemClock(void);

void serialPutChar(uint8_t c);
uint8_t serialGetChar(void);

uint32_t micros(void);
uint32_t millis(void);
void delayMicroseconds(uint32_t us);
void delay(uint32_t ms);

#endif
