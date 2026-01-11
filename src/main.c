// main.c
#include "PLL.h"
#include "SysTick.h"
#include "calculator.h"
#include "keypad.h"
#include "lcd.h"
#include "menu.h"
#include "password.h"

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

  int appState = 0; // 0=Auth, 1=Menu, 2=Calc

  while (1) {

    if (Password_IsUnlocked()) {
      if (appState == 0) {
        // Just unlocked. Go to Menu.
        appState = 1;
      }

      if (appState == 1) {
        // Show Menu
        int choice = Menu_Select();
        if (choice == 1) {
          appState = 2; // Calculator
          Calc_Reset(); // Prepare Calculator
        } else if (choice == 2) {
          // Assuming Tutorial_Show() exists in menu.h or another included file
          // If not, this line will cause a compilation error.
          // For this exercise, we assume it's available.
          Tutorial_Show();
          // Stay in Menu Loop (appState = 1)
        }
      } else if (appState == 2) {
        // Calculator Mode
        char key = readKeypad();
        if (key != 0) {
          char decoded = decodeKeyPress(key);
          // Option to exit back to menu?
          // Maybe Shift + # (Change Password) also exits?
          // For now, adhere to existing Calc logic.
          Calc_ProcessKey(decoded);

          // Debounce
          SysTick_Wait10ms(20);
          while (readKeypad() != 0)
            ; // Wait release
        }
      }

      // Check for Password Change (Shift+#) overrides
      // Logic inside Password_Change handles temporary takeover.
      // We might need to handle returning to Menu vs Calc.
      // Current Password_Change returns void.

    } else {
      // LOCKED STATE
      appState = 0;
      char key = readKeypad();
      if (key != 0) {
        char decoded = decodeKeyPress(key);
        Password_Check(decoded);

        // Debounce
        SysTick_Wait10ms(20);
        while (readKeypad() != 0)
          ;
      }
    }
  }
}
