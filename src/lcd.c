/*
 * File: lcd.c
 * Description: LCD Driver Implementation.
 */

#include "lcd.h"

// --- Register Definitions ---
// System Control
#define SYSCTL_RCGCGPIO_R (*((volatile unsigned long *)0x400FE608))
#define SYSCTL_PRGPIO_R (*((volatile unsigned long *)0x400FEA08))

// Port A (Control)
#define GPIO_PORTA_DIR_R (*((volatile unsigned long *)0x40004400))
#define GPIO_PORTA_AFSEL_R (*((volatile unsigned long *)0x40004420))
#define GPIO_PORTA_DEN_R (*((volatile unsigned long *)0x4000451C))

// Port B (Data)
#define GPIO_PORTB_DATA_R (*((volatile unsigned long *)0x400053FC))
#define GPIO_PORTB_DIR_R (*((volatile unsigned long *)0x40005400))
#define GPIO_PORTB_AFSEL_R (*((volatile unsigned long *)0x40005420))
#define GPIO_PORTB_DEN_R (*((volatile unsigned long *)0x4000551C))

// Bit-Specific Access (Masked Addresses)
#define LCD_EN_PIN (*((volatile unsigned long *)0x40004010)) 
#define LCD_RS_PIN (*((volatile unsigned long *)0x40004020))
#define LCD_DATA_PORT                                                          \
  (*((volatile unsigned long *)0x4000503C))

// --- Timing Functions ---

void lcdDelayUs(unsigned long us) {
  volatile unsigned long count = us * 13;
  while (count > 0) {
    count--;
  }
}

void lcdDelayMs(unsigned long ms) {
  while (ms > 0) {
    lcdDelayUs(1000);
    ms--;
  }
}


// Pulse the Enable (EN) pin to latch data
void lcdENPulse(void) {
  LCD_EN_PIN = 0x04; // Assert EN (High)
  lcdDelayUs(1);     // Hold > 450ns
  LCD_EN_PIN = 0x00; // De-assert EN (Low)
  lcdDelayUs(1);     // Hold > 450ns
}

// Send lower 4 bits of 'nibble' to LCD Data pins
void LCD_SendNibble(unsigned char nibble) {
  LCD_DATA_PORT = nibble & 0x0F;
  lcdENPulse();
}

// Send a full byte as two nibbles (High then Low)
void LCD_WriteByte(unsigned char data) {
  LCD_SendNibble((data >> 4) & 0x0F); // Upper nibble
  LCD_SendNibble(data & 0x0F);        // Lower nibble
  lcdDelayUs(37);                     // Default execution time
}

// --- Cursor Tracking ---
static unsigned char g_col = 0;
static unsigned char g_row = 0;

static void LCD_UpdateCursor(void) {
  unsigned char addr;
  switch (g_row) {
  case 0:
    addr = 0x00;
    break;
  case 1:
    addr = 0x40;
    break;
  case 2:
    addr = 0x14;
    break;
  case 3:
    addr = 0x54;
    break;
  default:
    addr = 0x00;
    break;
  }
  addr += g_col;
  lcdWriteCommand(0x80 | addr);
}

// --- Cursor Commands ---
void lcdCursorBlink(void) {
  lcdWriteCommand(0x0F); // Display On, Cursor On, Blink On
}

void lcdCursorOff(void) {
  lcdWriteCommand(0x0C); // Display On, Cursor Off, Blink Off
}

// --- Core Functions ---

void lcdWriteCommand(unsigned char c) {
  LCD_RS_PIN = 0x00; // RS Low (Command)
  LCD_WriteByte(c);
}


void lcdWriteData(char c) {
  LCD_RS_PIN = 0x08; // RS High (Data)
  LCD_WriteByte((unsigned char)c);
  LCD_RS_PIN = 0x00; // Cleanup

  g_col++;
  if (g_col >= 20) { // Assuming 20x4 display
    g_col = 0;
    g_row++;
    if (g_row >= 4) {
      g_row = 0; // Wrap back to top
    }
    LCD_UpdateCursor();
  }
}

// Clear the screen and reset cursor

void lcdClearScreen(void) {
  lcdWriteCommand(0x01);
  lcdDelayMs(2);
  g_col = 0;
  g_row = 0;
}

// Move cursor to specific DDRAM address

void lcdGoto(unsigned char address) {
  lcdWriteCommand(0x80 | (address & 0x7F));

  // Calculate Row/Col logic derived from address

  if (address >= 0x00 && address < 0x14) {
    g_row = 0;
    g_col = address;
  } else if (address >= 0x40 && address < 0x54) {
    g_row = 1;
    g_col = address - 0x40;
  } else if (address >= 0x14 && address < 0x28) {
    g_row = 2;
    g_col = address - 0x14;
  } else if (address >= 0x54 && address < 0x68) {
    g_row = 3;
    g_col = address - 0x54;
  } else {
    g_col = 0;
    g_row = 0;
  }
}
void printDisplay(char *str) {
  while (*str) {
    lcdWriteData(*str++);
  }
}

void lcdCreateCustomChar(unsigned char loc, unsigned char *pattern) {
  if (loc < 8) {
    int i;
    lcdWriteCommand(0x40 + (loc * 8));
    for (i = 0; i < 8; i++) {
      lcdWriteData(pattern[i]);
    }
  }
}

// Deletes the previous character (Backspace)
void lcdBackspace(void) {
  if (g_col > 0) {
    g_col--;
  } else {
    if (g_row > 0) {
      g_row--;
      g_col = 19; // Go to end of previous line (0-19)
    } else {
      return;
    }
  }

  // Move Cursor to new position
  LCD_UpdateCursor();

  LCD_RS_PIN = 0x08; // Data
  LCD_WriteByte(' ');
  LCD_RS_PIN = 0x00;

  LCD_UpdateCursor();
}

// --- Initialization ---

static void LCD_InitPorts(void) {
  volatile unsigned long delay;


  SYSCTL_RCGCGPIO_R |= 0x03;
  delay = SYSCTL_RCGCGPIO_R;


  GPIO_PORTB_DIR_R |= 0x0F;
  GPIO_PORTB_DEN_R |= 0x0F;
  GPIO_PORTB_AFSEL_R &= ~0x0F;


  GPIO_PORTA_DIR_R |= 0x0C;
  GPIO_PORTA_DEN_R |= 0x0C;
  GPIO_PORTA_AFSEL_R &= ~0x0C;


  LCD_RS_PIN = 0x00;
  LCD_EN_PIN = 0x00;
}

// Main Initialization Routine
void lcdInit(void) {
  LCD_InitPorts();

  lcdDelayMs(50);

  // Reset Sequence to ensure known state
  LCD_RS_PIN = 0x00;


  LCD_SendNibble(0x03);
  lcdDelayMs(5);
  LCD_SendNibble(0x03);
  lcdDelayUs(150);
  LCD_SendNibble(0x03);
  lcdDelayUs(150);

  // Switch to 4-bit Mode
  LCD_SendNibble(0x02);
  lcdDelayUs(150);

  // Configuration Commands
  // Function Set: 4-bit, 2-line, 5x8 dots
  lcdWriteCommand(0x28);

  lcdWriteCommand(0x08);

  lcdClearScreen(); 


  lcdWriteCommand(0x06);

  lcdWriteCommand(0x0C);
}
