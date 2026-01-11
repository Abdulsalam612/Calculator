/*
 * lcd.c
 * LCD Driver Implementation
 */

#include "lcd.h"

#define SYSCTL_RCGC2_R (*((volatile unsigned long *)0x400FE108))
#define GPIO_PORTA_AMSEL_R (*((volatile unsigned long *)0x40004528))
#define GPIO_PORTA_PCTL_R (*((volatile unsigned long *)0x4000452C))
#define GPIO_PORTA_DIR_R (*((volatile unsigned long *)0x40004400))
#define GPIO_PORTA_AFSEL_R (*((volatile unsigned long *)0x40004420))
#define GPIO_PORTA_DEN_R (*((volatile unsigned long *)0x4000451C))

#define GPIO_PORTB_AMSEL_R (*((volatile unsigned long *)0x40005528))
#define GPIO_PORTB_PCTL_R (*((volatile unsigned long *)0x4000552C))
#define GPIO_PORTB_DIR_R (*((volatile unsigned long *)0x40005400))
#define GPIO_PORTB_AFSEL_R (*((volatile unsigned long *)0x40005420))
#define GPIO_PORTB_DEN_R (*((volatile unsigned long *)0x4000551C))

// Delay function for timing
void DelayMs(unsigned long ms) {
  unsigned long i;
  while (ms > 0) {
    i = 13333;
    while (i > 0) {
      i = i - 1;
    }
    ms = ms - 1;
  }
}

void DelayUs(unsigned long us) {
  unsigned long i;
  while (us > 0) {
    i = 13;
    while (i > 0) {
      i = i - 1;
    }
    us = us - 1;
  }
}

void LCD_Init(void) {
  volatile unsigned long delay;

  // Enable Clock for Port A and Port B
  SYSCTL_RCGC2_R |= 0x03;
  delay = SYSCTL_RCGC2_R;

  // Setup Port A (Control: PA2, PA3)
  GPIO_PORTA_AMSEL_R &= ~0x0C;
  GPIO_PORTA_PCTL_R &= ~0x0000FF00;
  GPIO_PORTA_DIR_R |= 0x0C;
  GPIO_PORTA_AFSEL_R &= ~0x0C;
  GPIO_PORTA_DEN_R |= 0x0C;

  // Setup Port B (Data: PB0-PB3)
  GPIO_PORTB_AMSEL_R &= ~0x0F;
  GPIO_PORTB_PCTL_R &= ~0x0000FFFF;
  GPIO_PORTB_DIR_R |= 0x0F;
  GPIO_PORTB_AFSEL_R &= ~0x0F;
  GPIO_PORTB_DEN_R |= 0x0F;

  LCD_EN_PIN = 0;
  LCD_RS_PIN = 0;

  DelayMs(50); // Power up wait

  // Initialization Sequence
  LCD_OutNibble(0x03);
  DelayMs(5);
  LCD_OutNibble(0x03);
  DelayUs(100);
  LCD_OutNibble(0x03);
  DelayUs(100);
  LCD_OutNibble(0x02); // 4-bit mode
  DelayUs(100);

  // Configuration
  LCD_SendCmd(0x28); // Function Set: 4-bit, 2 lines, 5x7
  LCD_SendCmd(0x0C); // Display On, Cursor Off, Blink Off
  LCD_SendCmd(0x06); // Entry Mode: Increment, No Shift
  LCD_ClearScreen();
}

void LCD_OutNibble(byte nibble) {
  LCD_DATA_PORT = nibble & 0x0F;

  // Pulse Enable
  LCD_EN_PIN = 0x04; // PA2 High
  DelayUs(6);
  LCD_EN_PIN = 0x00; // PA2 Low
  DelayUs(6);
}

void LCD_SendCmd(byte cmd) {
  LCD_RS_PIN = 0;          // Command Mode
  LCD_OutNibble(cmd >> 4); // High nibble
  LCD_OutNibble(cmd);      // Low nibble
  DelayMs(2);
}

void LCD_SendData(byte data) {
  LCD_RS_PIN = 0x08; // Data Mode (PA3 High)
  LCD_OutNibble(data >> 4);
  LCD_OutNibble(data);
  DelayUs(50);
  LCD_RS_PIN = 0;
}

void LCD_ClearScreen(void) {
  LCD_SendCmd(0x01);
  DelayMs(2);
}

void LCD_ReturnHome(void) {
  LCD_SendCmd(0x02);
  DelayMs(2);
}

void LCD_SetCursor(byte r, byte c) {
  byte addr;
  if (r == 0)
    addr = 0x80 + c;
  else if (r == 1)
    addr = 0xC0 + c;
  else if (r == 2)
    addr = 0x94 + c;
  else
    addr = 0xD4 + c;

  LCD_SendCmd(addr);
}

void LCD_PrintChar(char c) { LCD_SendData((byte)c); }

void LCD_PrintString(char *s) {
  while (*s) {
    LCD_PrintChar(*s);
    s++;
  }
}

// Mapped functions for compatibility
void LCD_SetRegisterSelect(byte rs) {
  if (rs)
    LCD_RS_PIN = 0x08;
  else
    LCD_RS_PIN = 0;
}
void LCD_SendNibble(byte nibble) { LCD_OutNibble(nibble); }
void LCD_WriteByte(byte b) { LCD_SendData(b); }
void LCD_Initialize(void) { LCD_Init(); }
void LCD_Clear(void) { LCD_ClearScreen(); }
void LCD_Home(void) { LCD_ReturnHome(); }
void LCD_DisplayControl(byte d, byte c, byte b) {
  LCD_SendCmd(0x08 | (d << 2) | (c << 1) | b);
}
void LCD_ConfigureEntryMode(byte id, byte s) {
  LCD_SendCmd(0x04 | (id << 1) | s);
}
void LCD_ConfigureFunction(byte dl, byte n, byte f) {
  LCD_SendCmd(0x20 | (dl << 4) | (n << 3) | (f << 2));
}
void LCD_SetAddress(byte addr) { LCD_SendCmd(0x80 | addr); }
void LCD_SetCursorPosition(byte row, byte col) { LCD_SetCursor(row, col); }
void LCD_WriteChar(char ch) { LCD_PrintChar(ch); }
void LCD_WriteString(char str[]) { LCD_PrintString(str); }
