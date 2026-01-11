/*
 * lcd.c
 * LCD Driver Implementation
 *
 * Configures the TM4C123GH6PM to control an HD44780-based LCD via 4-bit mode.
 * Port A [3:2] -> Control (RS, EN)
 * Port B [3:0] -> Data (DB4-DB7)
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
#define LCD_EN_PIN (*((volatile unsigned long *)0x40004010)) // PA2 (0x04)
#define LCD_RS_PIN (*((volatile unsigned long *)0x40004020)) // PA3 (0x08)
#define LCD_DATA_PORT                                                          \
  (*((volatile unsigned long *)0x4000503C)) // PB0-PB3 (0x0F)

// --- Timing Functions ---

// Microsecond delay (approximate for 80MHz clock)
void lcdDelayUs(unsigned long us) {
  volatile unsigned long count = us * 13; // Calibrated for overhead
  while (count > 0) {
    count--;
  }
}

// Millisecond delay
void lcdDelayMs(unsigned long ms) {
  while (ms > 0) {
    lcdDelayUs(1000);
    ms--;
  }
}

// --- Low-Level Helpers ---

// Pulse the Enable (EN) pin to latch data
void lcdENPulse(void) {
  LCD_EN_PIN = 0x04; // Assert EN (High)
  lcdDelayUs(1);     // Hold > 450ns
  LCD_EN_PIN = 0x00; // De-assert EN (Low)
  lcdDelayUs(1);     // Hold > 450ns
}

// Send lower 4 bits of 'nibble' to LCD Data pins
void LCD_SendNibble(unsigned char nibble) {
  LCD_DATA_PORT = nibble & 0x0F; // Write to PB0-3
  lcdENPulse();                  // Latch the nibble
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

// Updates device cursor to match g_row/g_col
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

// --- Core Functions ---

// Send an Instruction/Command to the LCD
void lcdWriteCommand(unsigned char c) {
  LCD_RS_PIN = 0x00; // RS Low (Command)
  LCD_WriteByte(c);
}

// Send Data (Character) to the LCD
// Send Data (Character) to the LCD
void lcdWriteData(char c) {
  LCD_RS_PIN = 0x08; // RS High (Data)
  LCD_WriteByte((unsigned char)c);
  LCD_RS_PIN = 0x00; // Cleanup

  // Smart Line Wrapping
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
// Clear the screen and reset cursor
void lcdClearScreen(void) {
  lcdWriteCommand(0x01); // Clear Display Command
  lcdDelayMs(2);         // Requires > 1.5ms
  g_col = 0;
  g_row = 0;
}

// Move cursor to specific DDRAM address
// Move cursor to specific DDRAM address
void lcdGoto(unsigned char address) {
  // 0x80 is the "Set DDRAM Address" command base
  lcdWriteCommand(0x80 | (address & 0x7F));

  // Note: It's hard to reverse-engineer row/col from address perfectly
  // if custom addresses are used, but we reset tracking to safest guess
  // or user should rely on lcdClearScreen() to sync.
  // For now, we don't update g_row/g_col here to avoid breaking logic if
  // user knows what they are doing manually.
  // A better approach would be to calculate it:
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
  } // Fallback
}

// Print a null-terminated string
void printDisplay(char *str) {
  while (*str) {
    lcdWriteData(*str++);
  }
}

// Deletes the previous character (Backspace)
void lcdBackspace(void) {
  // 1. Decrement Cursor
  if (g_col > 0) {
    g_col--;
  } else {
    // Reverse Wrap
    if (g_row > 0) {
      g_row--;
      g_col = 19; // Go to end of previous line (0-19)
    } else {
      // Top-left corner, nowhere to go?
      // Optionally wrap to end of 4th line, but usually stick at 0,0
      return;
    }
  }

  // 2. Move Cursor to new position
  LCD_UpdateCursor();

  // 3. Overwrite with Space (This advances cursor)
  LCD_RS_PIN = 0x08; // Data
  LCD_WriteByte(' ');
  LCD_RS_PIN = 0x00;

  // 4. Move Cursor back again (because WriteByte advanced it)
  LCD_UpdateCursor();
}

// --- Initialization ---

// Initialize Port A and Port B for LCD Usage
static void LCD_InitPorts(void) {
  volatile unsigned long delay;

  // 1. Clock Enable
  SYSCTL_RCGCGPIO_R |= 0x03; // Enable Port A and B
  delay = SYSCTL_RCGCGPIO_R; // Delay for clock stab.

  // 2. Port B Config (Data: PB0-PB3)
  GPIO_PORTB_DIR_R |= 0x0F;    // Output
  GPIO_PORTB_DEN_R |= 0x0F;    // Digital Enable
  GPIO_PORTB_AFSEL_R &= ~0x0F; // GPIO

  // 3. Port A Config (Control: PA2, PA3)
  GPIO_PORTA_DIR_R |= 0x0C;    // Output
  GPIO_PORTA_DEN_R |= 0x0C;    // Digital Enable
  GPIO_PORTA_AFSEL_R &= ~0x0C; // GPIO

  // 4. Initial State
  LCD_RS_PIN = 0x00;
  LCD_EN_PIN = 0x00;
}

// Main Initialization Routine
void lcdInit(void) {
  LCD_InitPorts();

  lcdDelayMs(50); // Power-up wait (> 15ms)

  // Reset Sequence to ensure known state
  LCD_RS_PIN = 0x00; // Command Mode

  // Sync Sequence (0x03 sent 3 times)
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

  // Display Control: Display Off initially
  lcdWriteCommand(0x08);

  // Clear Screen
  lcdClearScreen(); // This logic now sets g_col=0, g_row=0

  // Entry Mode: Increment cursor, No Shift

  lcdWriteCommand(0x06);

  // Display Control: Display On, Cursor Off, Blink Off
  lcdWriteCommand(0x0C);
}
