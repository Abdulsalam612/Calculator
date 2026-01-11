// main.c
// Calculator Project

#include "PLL.h"
#include "SysTick.h"
#include "keypad.h"
#include "lcd.h"

int main(void) {
  // System Initialization (80 MHz)
  SysPLL_Init();
  SysTick_Init();

  // Initialize Drivers
  lcdInit();
  keypadInit();

  // Test: Print 'Salam'
  printDisplay("Salam");
  SysTick_Wait10ms(100); // Wait 1s
  lcdClearScreen();

  while (1) {
    unsigned char key = readKeypad();
    if (key != 0) {
      char c = decodeKeyPress(key);
      lcdWriteData(c);
      SysTick_Wait10ms(20); // Debounce 200ms
    }
  }
}
