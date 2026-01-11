/*
 * keypad.c
 * Keypad Driver Implementation
 *
 * Uses Port E (Rows) and Port D (Cols) to scan a 4x4 Matrix Keypad.
 * Rows: PE0-PE3
 * Cols: PD0-PD3
 */

#include "keypad.h"

// --- Register Definitions ---
// System Control
#define SYSCTL_RCGCGPIO_R (*((volatile unsigned long *)0x400FE608))
#define SYSCTL_PRGPIO_R (*((volatile unsigned long *)0x400FEA08))

// Port D (Columns)
#define GPIO_PORTD_DATA_R (*((volatile unsigned long *)0x400073FC))
#define GPIO_PORTD_DIR_R (*((volatile unsigned long *)0x40007400))
#define GPIO_PORTD_AFSEL_R (*((volatile unsigned long *)0x40007420))
#define GPIO_PORTD_DEN_R (*((volatile unsigned long *)0x4000751C))
#define GPIO_PORTD_PDR_R                                                       \
  (*((volatile unsigned long *)0x40007514)) // Pull-down resistor

// Port E (Rows)
#define GPIO_PORTE_DATA_R (*((volatile unsigned long *)0x400243FC))
#define GPIO_PORTE_DIR_R (*((volatile unsigned long *)0x40024400))
#define GPIO_PORTE_AFSEL_R (*((volatile unsigned long *)0x40024420))
#define GPIO_PORTE_DEN_R (*((volatile unsigned long *)0x4002451C))

// --- Core Functions ---

// Initializes Port D and Port E
void keypadInit(void) {
  volatile unsigned long delay;

  // 1. Activate Clock for Port D and Port E
  SYSCTL_RCGCGPIO_R |= 0x18; // Enable Port D (bit 3) and Port E (bit 4) -> 0x18
  delay = SYSCTL_RCGCGPIO_R; // Wait for clock

  // 2. Configure Port E (Rows 0-3) as Output
  GPIO_PORTE_AFSEL_R &= ~0x0F; // Disable alt function
  GPIO_PORTE_DIR_R |= 0x0F;    // PE0-PE3 Outputs
  GPIO_PORTE_DEN_R |= 0x0F;    // Enable Digital
  GPIO_PORTE_DATA_R &= ~0x0F;  // Init Low

  // 3. Configure Port D (Cols 0-3) as Input with Pull-Downs
  GPIO_PORTD_AFSEL_R &= ~0x0F; // Disable alt function
  GPIO_PORTD_DIR_R &= ~0x0F;   // PD0-PD3 Inputs
  GPIO_PORTD_DEN_R |= 0x0F;    // Enable Digital
  GPIO_PORTD_PDR_R |= 0x0F;    // Enable Pull-Down Resistors
}

// Scans the matrix
unsigned char readKeypad(void) {
  unsigned char row, col;

  // Loop through Rows (PE0-PE3)
  for (row = 0; row < 4; row++) {
    // Drive current row HIGH, others LOW
    GPIO_PORTE_DATA_R = (1 << row);

    // Brief delay for signal stabilization
    for (volatile int i = 0; i < 100; i++)
      ;

    // Read Columns (PD0-PD3)
    // Since we use Pull-Downs, pressed key will be HIGH
    col = GPIO_PORTD_DATA_R & 0x0F;

    if (col != 0) {
      // Key detected!
      // Return raw code: (Row 1-4) in upper nibble? Or simple linear index?
      // Let's return a composite byte: (Row << 4) | Col_Bitmask
      // e.g., Row 0 pressed, Col 2 (0x04) -> 0x04
      // Wait, proper decoding is easier if we return specific values.

      // Return unique code: Row(0-3) * 4 + Col_Index(0-3) + 1 ?
      // Let's stick to returning (Row index inside loop) and (Col data)

      // Clean up row line
      GPIO_PORTE_DATA_R = 0;
      return (row << 4) | col;
    }
  }

  return 0; // No key pressed
}

// Decodes raw key code to ASCII
char decodeKeyPress(unsigned char k) {
  unsigned char row = (k >> 4);
  unsigned char colData = (k & 0x0F);
  unsigned char col = 0;

  if (k == 0)
    return 0;

  // Resolve column bitmask to index
  if (colData == 0x01)
    col = 0;
  else if (colData == 0x02)
    col = 1;
  else if (colData == 0x04)
    col = 2;
  else if (colData == 0x08)
    col = 3;
  else
    return 0; // Error or multi-press

  // Standard 4x4 Keypad Map
  // 1 2 3 A
  // 4 5 6 B
  // 7 8 9 C
  // * 0 # D

  const char keyMap[4][4] = {{'1', '2', '3', 'A'},
                             {'4', '5', '6', 'B'},
                             {'7', '8', '9', 'C'},
                             {'*', '0', '#', 'D'}};

  return keyMap[row][col];
}
