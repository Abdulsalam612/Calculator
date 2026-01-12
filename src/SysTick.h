/*
 * File: SysTick.h
 * Description: Public interface for SysTick delays.
 */

#ifndef SYSTICK_H
#define SYSTICK_H

void SysTick_Init(void);

void SysTick_Wait(unsigned long delay);


void SysTick_Wait10ms(unsigned long delay);

#endif /* SYSTICK_H_ */
