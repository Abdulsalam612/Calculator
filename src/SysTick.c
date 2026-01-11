/*
 * SysTick.c
 * SysTick Driver Implementation
 */

#include "SysTick.h"

#define NVIC_ST_CTRL_R (*((volatile unsigned long *)0xE000E010))
#define NVIC_ST_RELOAD_R (*((volatile unsigned long *)0xE000E014))
#define NVIC_ST_CURRENT_R (*((volatile unsigned long *)0xE000E018))

void SysTick_Init(void) {
  NVIC_ST_CTRL_R = 0;            // disable SysTick during setup
  NVIC_ST_RELOAD_R = 0x00FFFFFF; // maximum reload value
  NVIC_ST_CURRENT_R = 0;         // any write to current clears it
  NVIC_ST_CTRL_R = 0x00000005;   // enable SysTick with core clock
}

// Time delay using busy wait.
// The delay parameter is in units of the 80 MHz core clock. (12.5 ns)
void SysTick_Wait(unsigned long delay) {
  volatile unsigned long elapsedTime;
  unsigned long startTime = NVIC_ST_CURRENT_R;
  do {
    elapsedTime = (startTime - NVIC_ST_CURRENT_R) & 0x00FFFFFF;
  } while (elapsedTime <= delay);
}

// 800,000 * 12.5ns equals 10ms
void SysTick_Wait10ms(unsigned long delay) {
  unsigned long i;
  for (i = 0; i < delay; i++) {
    SysTick_Wait(800000); // wait 10ms
  }
}
