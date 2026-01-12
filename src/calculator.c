/*
 * File: calculator.c
 * Description: Calculator Core Implementation.
 Using a Shunting-yard algorithm (;
 */

#include "calculator.h"
#include "lcd.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_EXPR_LEN 64
#define MAX_STACK 32

// State Management
static char g_inputBuffer[MAX_EXPR_LEN];
static int g_bufferIndex = 0;
static int g_resetOnNextKey = 0;

static int g_shiftActive = 0;  // 0=Off, 1=On
static double g_lastAns = 0.0; // Store last result

// Stacks for Evaluation
static double valStack[MAX_STACK];
static int valTop = -1;
static char opStack[MAX_STACK];
static int opTop = -1;

// Helper for isdigit (implementation)
int my_isdigit(char c) { return (c >= '0' && c <= '9'); }

int is_operator(char c) {
  return (c == '+' || c == '-' || c == '*' || c == '/' || c == '^');
}

// 0 = OK, 1 = Error
int ValidateSyntax(void) {
  int i;
  int lastIsOp = 0; // 0=No, 1=Yes
  int lastIsDot = 0;

  if (g_bufferIndex == 0)
    return 0; // Empty is safe (ignores #)

  // Check start
  if (is_operator(g_inputBuffer[0]) && g_inputBuffer[0] != '-') {
    return 1;
  }

  for (i = 0; i < g_bufferIndex; i++) {
    char c = g_inputBuffer[i];

    if (my_isdigit(c)) {
      lastIsOp = 0;
      lastIsDot = 0;
    } else if (c == '.') {
      if (lastIsDot)
        return 1; // .. Error
      lastIsDot = 1;
      lastIsOp = 0;
    } else if (is_operator(c)) {
      if (lastIsOp)
        return 1; // ** Error
      lastIsOp = 1;
      lastIsDot = 0;
    } else {
    }
  }

  // Ends with operator. 5+ is error
  if (lastIsOp)
    return 1;

  return 0;
}

// --- Helper Functions ---
void Calc_Reset(void) {
  g_bufferIndex = 0;
  memset(g_inputBuffer, 0, MAX_EXPR_LEN);
  lcdClearScreen();
  lcdClearScreen();

  g_resetOnNextKey = 0;
  g_shiftActive = 0;
  lcdCursorBlink(); // Ready for input
}

// Push value
void pushVal(double v) {
  if (valTop < MAX_STACK - 1)
    valStack[++valTop] = v;
}

// Pop value
double popVal(void) {
  if (valTop >= 0)
    return valStack[valTop--];
  return 0.0;
}

// Push Op
void pushOp(char op) {
  if (opTop < MAX_STACK - 1)
    opStack[++opTop] = op;
}

// Pop Op
char popOp(void) {
  if (opTop >= 0)
    return opStack[opTop--];
  return '\0';
}

// Get Precedence
int precedence(char op) {
  if (op == '+' || op == '-')
    return 1;
  if (op == '*' || op == '/')
    return 2;
  if (op == '^')
    return 3; // Power has higher precedence
  return 0;
}

// Power Function
double calc_pow(double base, double exp) {
  double res = 1.0;
  int i;
  for (i = 0; i < (int)exp; i++)
    res *= base;
  return res;
}

// Apply Operation
double applyOp(double a, double b, char op) {
  switch (op) {
  case '+':
    return a + b;
  case '-':
    return a - b;
  case '*':
    return a * b;
  case '/':
    return (b != 0) ? (a / b) : 0.0; // Avoid DivByZero crash
  case '^':
    return calc_pow(a, b);
  default:
    return 0.0;
  }
}

// Evaluate the buffered string
void Calc_Evaluate(void) {
  if (ValidateSyntax()) {
    lcdClearScreen();
    printDisplay("Syntax Error");
    g_resetOnNextKey = 1;
    return;
  }

  int i;

  valTop = -1;
  opTop = -1;

  // Simple Parser
  for (i = 0; i < g_bufferIndex; i++) {
    // Skip whitespace
    if (g_inputBuffer[i] == ' ')
      continue;

    // If Digit or Decimal point, parse number
    if (my_isdigit(g_inputBuffer[i]) || g_inputBuffer[i] == '.') {
      char numStr[32];
      int k = 0;
      // Capture full number
      while (i < g_bufferIndex &&
             (my_isdigit(g_inputBuffer[i]) || g_inputBuffer[i] == '.')) {

        numStr[k++] = g_inputBuffer[i++];
      }
      numStr[k] = '\0';
      i--; // Backtrack one step as loop increments

      pushVal(atof(numStr));
    } else {
      // It's an operator
      char currentOp = g_inputBuffer[i];

      while (opTop != -1 &&
             precedence(opStack[opTop]) >= precedence(currentOp)) {
        double val2 = popVal();
        double val1 = popVal();
        char op = popOp();
        pushVal(applyOp(val1, val2, op));
      }
      pushOp(currentOp);
    }
  }

  // Apply remaining ops
  while (opTop != -1) {
    double val2 = popVal();
    double val1 = popVal();
    char op = popOp();
    pushVal(applyOp(val1, val2, op));
  }

  // Result is at top of valStack
  double result = popVal();

  // Format Result String

  char outStr[32];

  // Check if integer
  if (result == (long)result) {
    sprintf(outStr, "= %ld", (long)result);
  } else {
    sprintf(outStr, "= %.3f", result);
  }

  // Store Result in Ans
  g_lastAns = result;

  lcdCursorOff();       // Hide cursor while showing result
  lcdWriteData(' ');    // Space before equals
  printDisplay(outStr);

  g_resetOnNextKey = 1; // Flag to clear on next input
}

// --- Public Interface ---
void Calc_Init(void) { Calc_Reset(); }

int Calc_IsShiftActive(void) { return g_shiftActive; }

void Calc_ProcessKey(char key) {

  if (g_resetOnNextKey) {
    if (key == '#')
      return; // Ignore repeated equals
    Calc_Reset();
  }

  // Input Key Handling
  if (key == '#') {
    // Evaluate
    Calc_Evaluate();
    return;
  }

  // Handle Backspace ('*')
  if (key == '*') {
    if (g_bufferIndex > 0) {
      g_bufferIndex--;
      g_inputBuffer[g_bufferIndex] = '\0';
      lcdBackspace();
    }
    return;
  }

  // Map Keypad Chars to Operators
  char displayChar = key;
  char bufferChar = key;


  // Reset defaults for this key
  displayChar = key;
  bufferChar = key;

  // Handle Shift Key Toggle ('D')

  if (key == 'D') {
    g_shiftActive = !g_shiftActive;
    // Toggle Shift
    return;
  }

  if (g_shiftActive) {
    // SHIFT MAPPINGS
    switch (key) {
    case '0':
      bufferChar = '.';
      displayChar = '.';
      break; // Shift+0 = Dot

    case 'A':
      // Shift+A = Ans
      {
        char ansStr[32];
        int len;
        if (g_lastAns == (long)g_lastAns)
          sprintf(ansStr, "%ld", (long)g_lastAns);
        else
          sprintf(ansStr, "%.3f", g_lastAns);

        len = strlen(ansStr);

        // Allow if buffer fits
        if (g_bufferIndex + len < MAX_EXPR_LEN) {
          strcpy(&g_inputBuffer[g_bufferIndex], ansStr);
          g_bufferIndex += len;
          printDisplay("Ans");
          g_shiftActive = 0; // Auto-untoggle
          return;
        }
        g_shiftActive = 0; // Auto-untoggle even if full
        return;
      }

    case 'B':
      bufferChar = '^';
      displayChar = '^';
      break; // Shift+B = Power

    case 'C':
      bufferChar = '/';
      displayChar = '/';
      break; // Shift+C = Divide
    default:
      break;
    }




    // Auto-untoggle Shift
    g_shiftActive = 0;


  } else {

    // NORMAL MAPPINGS
    switch (key) {
    case 'A':
      bufferChar = '+';
      displayChar = '+';
      break;
    case 'B':
      bufferChar = '-';
      displayChar = '-';
      break;
    case 'C':
      bufferChar = '*';
      displayChar = '*';
      break;
    default:
      break;
    }
  }



  if (g_bufferIndex < MAX_EXPR_LEN - 1) {
    g_inputBuffer[g_bufferIndex++] = bufferChar;
    g_inputBuffer[g_bufferIndex] = '\0';

    lcdWriteData(displayChar);
  }
}
