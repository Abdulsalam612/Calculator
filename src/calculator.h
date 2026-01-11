/*
 * calculator.h
 * Calculator Logic Header
 * Handles input buffering and expression evaluation.
 */

#ifndef CALCULATOR_H_
#define CALCULATOR_H_

// Resets the calculator state
void Calc_Init(void);

// Processes a key press from the keypad
void Calc_ProcessKey(char key);

#endif /* CALCULATOR_H_ */
