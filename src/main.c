// main.c
// Calculator Proje#include "keypad.h"
#include "PLL.h"
#include "SysTick.h"
#include "calculator.h"

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

  // Intro
  printDisplay("Salam Calculator");
  SysTick_Wait10ms(200); // 2s
  lcdClearScreen();

  while (1) {
    unsigned char key = readKeypad();
    if (key != 0) {
      char c = decodeKeyPress(key);
      Calc_ProcessKey(c);   // Pass to Calculator Engine
      SysTick_Wait10ms(30); // Debounce 300ms
    }
  }
}
