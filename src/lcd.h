/*
 * File: lcd.h
 * Description: Public interface for the LCD driver.
 */

#ifndef LCD_H
#define LCD_H

/* Hardware Connections */
/*
 * Control Lines (Port A):
 * EN -> PA2
 * RS -> PA3
 */
#define LCD_EN_PIN (*((volatile unsigned long *)0x40004010))
#define LCD_RS_PIN (*((volatile unsigned long *)0x40004020))

/*
 * Data Lines (Port B):
 * DB4-DB7 -> PB0-PB3
 */
#define LCD_DATA_PORT (*((volatile unsigned long *)0x4000503C))

/* Function Prototypes */
void lcdInit(void);
void lcdWriteCommand(unsigned char c);
void lcdWriteData(char c);
void lcdClearScreen(void);
void lcdGoto(unsigned char address);
void printDisplay(char *str);
void lcdBackspace(void);

void lcdCursorBlink(void);
void lcdCursorOff(void);

// Create Custom Character (0-7 locations)
// pattern must be 8 bytes
void lcdCreateCustomChar(unsigned char loc, unsigned char *pattern);

void lcdDelayMs(unsigned long ms);

/* Helper / Extra Functions */
void lcdENPulse(void);

#endif /* LCD_H_ */
