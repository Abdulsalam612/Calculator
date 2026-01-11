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

  // Intro (Show this FIRST)
  printDisplay("Salam Calculator");
  SysTick_Wait10ms(200); // 2s

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
