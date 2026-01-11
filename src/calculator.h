/*
 * calculator.h
 * Calculator Logic Header
 * Handles input buffering and expression evaluation.
 */

#ifndef CALCULATOR_H_
#define CALCULATOR_H_

// Initialize Calculator (Same as Reset)
void Calc_Init(void);
void Calc_Reset(void);

// Process Key Input
void Calc_ProcessKey(char key);

// Check if Shift is Active
int Calc_IsShiftActive(void);

#endif /* CALCULATOR_H_ */
