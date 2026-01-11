/*
 * SysTick.h
 * SysTick Driver Header File
 * Provides precise busy-wait delays.
 */

#ifndef SYSTICK_H_
#define SYSTICK_H_

// Initialize SysTick periodically or just for delays?
// For this project, we primarily need it for Delays.
void SysTick_Init(void);

// Wait for 'delay' number of clock cycles (12.5 ns units at 80 MHz)
void SysTick_Wait(unsigned long delay);

// Wait for 'delay' number of 10ms units
void SysTick_Wait10ms(unsigned long delay);

#endif /* SYSTICK_H_ */
