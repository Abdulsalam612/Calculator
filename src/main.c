// main.c
// Calculator Project

#include "TExaS.h"
#include "keypad.h"
#include "lcd.h"

int main(void) {
  // Initialize LCD
  lcdInit();
  keypadInit();

  // Test: Print 'Salam'
  printDisplay("Salam");
  lcdDelayMs(1000);
  lcdClearScreen();

  while (1) {
    unsigned char key = readKeypad();
    if (key != 0) {
      char c = decodeKeyPress(key);
      lcdWriteData(c);
      lcdDelayMs(200); // Debounce / Avoid repeat
    }
  }
}
