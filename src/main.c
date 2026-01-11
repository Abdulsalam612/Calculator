

// main.c
// Calculator Proje#include "keypad.h"
#include "PLL.h"
#include "SysTick.h"
#include "calculator.h"
#include "password.h"

#include "keypad.h"
#include "lcd.h"

int main(void) {
  // System Initialization (80 MHz)
  SysPLL_Init();
  SysTick_Init();

  // Initialize Drivers
  lcdInit();
  keypadInit();
  Calc_Init();

  // Intro: Loading Animation
  lcdClearScreen();
  printDisplay("Loading...");
  lcdGoto(0x40); // 2nd Line

  // Progress Bar Animation
  int i;
  for (i = 0; i < 16; i++) {
    lcdWriteData(0xFF); // Display Black Block (Standard on HD44780)
    // If 0xFF doesn't work on this specific display, it might show garbage.
    // But usually it's a block. If not, we can change to '#'.
    SysTick_Wait10ms(5); // 50ms per block -> ~0.8s total
  }
  SysTick_Wait10ms(50); // Hold for 0.5s

  // Then Lock

  Password_Init(); // Clears screen and shows LOCKED

  while (1) {
    unsigned char key = readKeypad();
    if (key != 0) {
      char c = decodeKeyPress(key);

      if (Password_IsUnlocked()) {
        // Check for Change Password Command: Shift + #
        if (c == '#' && Calc_IsShiftActive()) {
          Password_Change();
        } else {
          Calc_ProcessKey(c); // Normal Calculator Mode
        }
      } else {
        Password_Check(c); // Locked Mode
      }

      SysTick_Wait10ms(30); // Debounce
    }
  }
}
