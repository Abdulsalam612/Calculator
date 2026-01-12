/*
 * File: menu.c
 * Description: Implementation of the Main Menu and Tutorial system.
 */

#include "menu.h"

#include "SysTick.h"
#include "keypad.h"
#include "lcd.h"

// Helper to display a page and wait for navigation
// Returns 1 to continue, 0 to exit (if * is pressed)
// Returns: 1=Next, -1=Prev, 0=Exit
int Tutorial_Page(char *title, char *l1, char *l2, int pageNum) {
  lcdClearScreen();
  printDisplay(title);

  if (l1) {
    lcdGoto(0x40); // Line 2
    printDisplay(l1);
  }
  if (l2) {
    lcdGoto(0x14); // Line 3
    printDisplay(l2);
  }

  // Footer (Line 4)
  lcdGoto(0x54);
  printDisplay("      Page ");
  lcdWriteData('0' + pageNum);

  // Wait for key press
  while (1) {
    unsigned char k = readKeypad();
    if (k != 0) {
      char c = decodeKeyPress(k);

      // Wait for release
      while (readKeypad() != 0)
        ;

      if (c == '0')
        return 0; // Exit
      if (c == '#')
        return 1; // Next
      if (c == '*')
        return -1; // Prev
    }
    SysTick_Wait10ms(5);
  }
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
  int page = 1;
  int result = 0;

  while (page >= 1 && page <= 5) {
    switch (page) {
    case 1:
      // Controls Page
      result = Tutorial_Page("Tutorial Controls", "*:Back #:Next", "0:Exit", 1);
      break;
    case 2:
      // Basic Ops
      result = Tutorial_Page("Basic Keys", "A:+ B:- C:*", "D:Shift", 2);
      break;
    case 3:
      // Control Keys
      result = Tutorial_Page("Other Keys", "*:Backspace", "#:Evaluate", 3);
      break;
    case 4:
      // Shift Ops 1
      result =
          Tutorial_Page("Shift Ops 1", "Sh+A:Ans Sh+B:^", "Sh+C:Div (/)", 4);
      break;
    case 5:
      // Shift Ops 2
      result =
          Tutorial_Page("Shift Ops 2", "Sh+0:Dot (.)", "Sh+#:Change PIN", 5);
      break;
    }

    if (result == 0)
      break; // Exit
    if (result == 1)
      page++;
    if (result == -1)
      page--;

    // Boundary
    if (page < 1)
      page = 1; // Can't go before 1
  }

  // End
  lcdClearScreen();
  lcdCursorOff();
  printDisplay("Exiting Tutorial...");
  SysTick_Wait10ms(100);
}
