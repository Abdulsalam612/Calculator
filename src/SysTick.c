/*
 * File: SysTick.c
 * Description: SysTick Timer Driver for generating precise delays.
 */

#include "SysTick.h"

#define NVIC_ST_CTRL_R (*((volatile unsigned long *)0xE000E010))
#define NVIC_ST_RELOAD_R (*((volatile unsigned long *)0xE000E014))
#define NVIC_ST_CURRENT_R (*((volatile unsigned long *)0xE000E018))

void SysTick_Init(void) {
  NVIC_ST_CTRL_R = 0;
  NVIC_ST_RELOAD_R = 0x00FFFFFF; // maximum reload value
  NVIC_ST_CURRENT_R = 0;         // any write to current clears it
  NVIC_ST_CTRL_R = 0x00000005;   // enable SysTick core clock
}

// Time delay using busy wait.
//
void SysTick_Wait(unsigned long delay) {
  volatile unsigned long elapsedTime;
  unsigned long startTime = NVIC_ST_CURRENT_R;
  do {
    elapsedTime = (startTime - NVIC_ST_CURRENT_R) & 0x00FFFFFF;
  } while (elapsedTime <= delay);
}

// 800000 * 12.5ns is  10ms
void SysTick_Wait10ms(unsigned long delay) {
  unsigned long i;
  for (i = 0; i < delay; i++) {
    SysTick_Wait(800000); // wait 10ms
  }
}
