/*
 * File: menu.c
 * Description: Implementation of the Main Menu and Tutorial system.
 */

#include "menu.h"

#include "SysTick.h"
#include "keypad.h"
#include "lcd.h"

// Helper to wait for any key press to continue
void waitForContinue(void) {
  lcdGoto(0x54); // Line 4
  printDisplay("Press Any Key...");

  // Wait for key press
  while (readKeypad() == 0)
    ;
  // Wait for release
  while (readKeypad() != 0)
    ;

  lcdClearScreen();
}

int Menu_Select(void) {
  lcdClearScreen();
  lcdCursorOff();
  printDisplay("--- Main Menu ---");
  lcdGoto(0x40); // Line 2
  printDisplay("1. Calculator");
  lcdGoto(0x14); // Line 3
  printDisplay("2. Tutorial");
  lcdGoto(0x54); // Line 4
  printDisplay("Select Option [1-2]");

  while (1) {
    unsigned char k = readKeypad();
    if (k != 0) {
      char c = decodeKeyPress(k);
      if (c == '1') {
        while (readKeypad() != 0)
          ; // Wait Release
        return 1;
      }
      if (c == '2') {
        while (readKeypad() != 0)
          ; // Wait Release
        return 2;
      }
    }
    SysTick_Wait10ms(5);
  }
}

void Tutorial_Show(void) {
  // Page 1: Basics
  lcdClearScreen();
  printDisplay("--- Tutorial 1/3 ---");
  lcdGoto(0x40);
  printDisplay("* = Backspace");
  lcdGoto(0x14);
  printDisplay("D = Shift (Toggle)");
  waitForContinue();

  // Page 2: Shift Functions
  lcdClearScreen();
  printDisplay("--- Tutorial 2/3 ---");
  lcdGoto(0x40);
  printDisplay("Shift+0 = .(Dot)");
  lcdGoto(0x14);
  printDisplay("Shift+A = Ans");
  waitForContinue();

  // Page 3: More Shift
  lcdClearScreen();
  printDisplay("--- Tutorial 3/3 ---");
  lcdGoto(0x40);
  printDisplay("Shift+B = Power (^)");
  lcdGoto(0x14);
  printDisplay("Shift+C = Divide (/)");
  waitForContinue();

  // End
  lcdClearScreen();
  printDisplay("End of Tutorial");
  SysTick_Wait10ms(100);
}
