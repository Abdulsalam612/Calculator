/*
 * lcd.h
 * LCD Driver Header File
 */

#ifndef LCD_H_
#define LCD_H_

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

/* Helper / Extra Functions */
void lcdENPulse(void);

#endif /* LCD_H_ */
