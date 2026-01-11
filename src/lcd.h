/*
 * lcd.h
 * LCD Driver Header File
 */

#ifndef LCD_H_
#define LCD_H_

#include <stdbool.h>

/* Type definitions */
typedef unsigned char byte;

/* Hardware Connections */
/* 
 * Control Lines (Port A):
 * EN -> PA2
 * RS -> PA3
 */
#define LCD_EN_PIN  (*((volatile unsigned long *)0x40004010)) 
#define LCD_RS_PIN  (*((volatile unsigned long *)0x40004020))

/* 
 * Data Lines (Port B):
 * DB4-DB7 -> PB0-PB3 
 */
#define LCD_DATA_PORT (*((volatile unsigned long *)0x4000503C))

/* Display Dimensions */
#define ROWS 4
#define COLS 20

/* Core Interface */
void LCD_Init(void);
void LCD_SendCmd(byte cmd);
void LCD_SendData(byte data);

/* Low Level Helpers */
void LCD_OutNibble(byte nibble);
void LCD_SetRS(byte state);

/* High Level Control */
void LCD_ClearScreen(void);
void LCD_ReturnHome(void);
void LCD_configEntry(byte id, byte s);
void LCD_configDisplay(byte d, byte c, byte b);
void LCD_configFunc(byte dl, byte n, byte f);

/* Cursor Management */
void LCD_SetCursor(byte r, byte c);
void LCD_SetDDRAM(byte addr);

/* String Output */
void LCD_PrintChar(char c);
void LCD_PrintString(char *s);

#endif /* LCD_H_ */
