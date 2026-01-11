/*
 * File: password.c
 * Description: Password management module. Handles PIN validation,
 *              storage in Flash, and access control logic.
 */

#include "Flash.h"

#include "SysTick.h"
#include "keypad.h" // Needed for blocking input
#include "lcd.h"

#include <stdio.h>
#include <stdlib.h> // for atoi/strtoul if needed
#include <string.h>

static int g_isUnlocked = 0;

static char g_enteredPin[5]; // 4 digits + null
static int g_pinIndex = 0;
static char g_correctPin[5] = "1234"; // Default PIN (non-const to allow change)

void Password_Init(void) {
  g_isUnlocked = 0;
  g_pinIndex = 0;
  memset(g_enteredPin, 0, sizeof(g_enteredPin));

  Flash_Init();
  // Load from Flash
  uint32_t storedData = Flash_Read(FLASH_PASSWORD_ADDR);

  // 0xFFFFFFFF means erased. Use default.
  if (storedData == 0xFFFFFFFF) {
    strcpy(g_correctPin, "1234");
  } else {
    // Unpack 4 bytes
    // Retrieve PIN from stored word

    memcpy(g_correctPin, &storedData, 4);
    g_correctPin[4] = '\0';

    // Validation: Check if it's actually 4 digits
    int isValid = 1;
    int i;
    for (i = 0; i < 4; i++) {
      if (g_correctPin[i] < '0' || g_correctPin[i] > '9') {
        isValid = 0;
        break;
      }
    }

    if (!isValid) {
      strcpy(g_correctPin, "1234");
    }
  }

  // Load Custom Chars
  // Lock Icon (Location 0)
  uint8_t lockChar[] = {0x0E, //  xxx
                        0x11, // x   x
                        0x11, // x   x
                        0x1F, // xxxxx
                        0x1B, // xx xx
                        0x1B, // xx xx
                        0x1F, // xxxxx
                        0x00};
  lcdCreateCustomChar(0, lockChar);

  // Unlock Icon (Location 1)
  uint8_t unlockChar[] = {0x0E, //  xxx
                          0x01, //     x
                          0x01, //     x
                          0x1F, // xxxxx
                          0x1B, // xx xx
                          0x1B, // xx xx
                          0x1F, // xxxxx
                          0x00};
  lcdCreateCustomChar(1, unlockChar);

  lcdClearScreen();
  lcdCursorOff(); // Hide cursor on title screen

  printDisplay("--- LOCKED ");
  lcdWriteData(0); // Show Lock Icon

  printDisplay(" ---");

  lcdGoto(0x40); // Line 2
  printDisplay("Enter PIN:");
  lcdGoto(0x14);    // Line 3
  lcdCursorBlink(); // Show cursor for PIN input
}

int Password_IsUnlocked(void) { return g_isUnlocked; }

void Password_Lock(void) { Password_Init(); }

void Password_Check(char key) {
  if (g_isUnlocked)
    return;

  // Only accept digits 0-9
  if (key >= '0' && key <= '9') {
    if (g_pinIndex < 4) {
      g_enteredPin[g_pinIndex++] = key;
      g_enteredPin[g_pinIndex] = '\0';

      // Visual Feedback (Masked or Number)
      lcdWriteData('*');
    }
  }
  // Handle Delete (Backspace)

  else if (key == '*') {
    if (g_pinIndex > 0) {
      g_pinIndex--;
      g_enteredPin[g_pinIndex] = '\0';
      lcdBackspace();
    }
  }
  // Enter / Confirm (#)
  else if (key == '#') {
    if (strcmp(g_enteredPin, g_correctPin) == 0) {
      g_isUnlocked = 1;
      lcdClearScreen();
      lcdCursorOff(); // Hide during message
      printDisplay("Access Granted! ");
      lcdWriteData(1);       // Show Unlock Icon
      SysTick_Wait10ms(100); // 1s delay
      lcdClearScreen();      // Empty Canvas
      lcdCursorBlink();      // Ready for Calc

    } else {
      lcdClearScreen();
      lcdCursorOff(); // Hide during message
      printDisplay("Wrong PIN!");
      SysTick_Wait10ms(100); // 1s delay

      // Reset
      g_pinIndex = 0;
      memset(g_enteredPin, 0, sizeof(g_enteredPin));
      lcdClearScreen();
      printDisplay("--- LOCKED ---");
      lcdGoto(0x40);
      printDisplay("Enter PIN:");
      lcdGoto(0x14);
    }
  }
}

// Blocking function to change password
void Password_Change(void) {
  char newPin[5];
  int idx = 0;

  lcdClearScreen();
  printDisplay("New PIN:");
  lcdGoto(0x40);

  // Simple blocking loop
  while (1) {
    unsigned char k = readKeypad();
    if (k != 0) {
      char c = decodeKeyPress(k); // We assume keypad.h is available

      if (c >= '0' && c <= '9') {
        if (idx < 4) {
          newPin[idx++] = c;
          lcdWriteData(c); // Show number
        }
      } else if (c == '*' && idx > 0) { // Backspace
        idx--;
        lcdBackspace();
      } else if (c == '#' && idx == 4) { // Confirm
        newPin[4] = '\0';
        strcpy(g_correctPin, newPin); // Store new PIN

        // Save to Flash
        Flash_Erase(FLASH_PASSWORD_ADDR);
        // Pack 4 bytes into uint32
        uint32_t data = 0;
        memcpy(&data, newPin, 4);
        Flash_Write(FLASH_PASSWORD_ADDR, data);

        lcdClearScreen();
        lcdCursorOff(); // Hide during message
        printDisplay("PIN Changed!");
        SysTick_Wait10ms(100);

        // Return to Calc
        lcdClearScreen(); // Empty Canvas
        lcdCursorBlink(); // Ready for Calc
        return;
      }

      SysTick_Wait10ms(30); // Debounce

      while (readKeypad() != 0)
        ; // Wait Release
    }
  }
}
