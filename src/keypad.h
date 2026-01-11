/*
 * keypad.h
 * Keypad Driver Header File
 */

#ifndef KEYPAD_H_
#define KEYPAD_H_

// Initializes Port D and Port E for 4x4 Keypad scanning
void keypadInit(void);

// Scans the keypad and returns the unique key code (or 0 if none)
unsigned char readKeypad(void);

// Converts the unique key code to an ASCII character
char decodeKeyPress(unsigned char k);

#endif /* KEYPAD_H_ */
