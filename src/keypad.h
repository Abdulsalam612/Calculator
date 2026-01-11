/*
 * File: keypad.h
 * Description: Public interface for the Keypad driver.
 */

#ifndef KEYPAD_H
#define KEYPAD_H

// Initializes Port D and Port E for 4x4 Keypad scanning
void keypadInit(void);

// Scans the keypad and returns the unique key code (or 0 if none)
unsigned char readKeypad(void);

// Converts the unique key code to an ASCII character
char decodeKeyPress(unsigned char k);

#endif /* KEYPAD_H_ */
