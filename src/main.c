/*
 * File: main.c
 * Description: Main application
 */

#include "PLL.h"
#include "SysTick.h"
#include "calculator.h"
#include "keypad.h"
#include "lcd.h"
#include "menu.h"
#include "password.h"

int main(void) {
  // System Initialization
  SysPLL_Init();
  SysTick_Init();

  // Initialize Drivers
  lcdInit();
  keypadInit();
  Calc_Init();

  // Intro: Loading Animation
  lcdClearScreen();
  lcdCursorOff(); // Ensure cursor is off
  printDisplay("Loading...");
  lcdGoto(0x40); // 2nd Line

  // Progress Bar Animation
  int i;
  for (i = 0; i < 60; i++) {
    lcdWriteData(0xFF);  // Display Block Character
    SysTick_Wait10ms(2); // 20ms per block -> 1.2s total
  }
  SysTick_Wait10ms(50);

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
          Tutorial_Show();
          // Return to Menu Loop
        }
      } else if (appState == 2) {
        // Calculator Mode
        char key = readKeypad();
        if (key != 0) {
          char decoded = decodeKeyPress(key);
          // Pass key to calculator submodule

          // Check for Shortcuts
          if (decoded == '#' && Calc_IsShiftActive()) {
            Password_Change();
            Calc_Reset(); // Restore Calculator UI after return
          } else {
            Calc_ProcessKey(decoded);
          }

          // Debounce
          SysTick_Wait10ms(20);
          while (readKeypad() != 0)
            ; // Wait release
        }
      }

      // Check for Password Change 

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
