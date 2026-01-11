/*
 * calculator.c
 * Calculator Core Implementation
 * Uses a Two-Stack Algorithm for operator precedence.
 */

#include "calculator.h"
#include "lcd.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// #include <ctype.h> // Not found in some embedded envs

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

// Helper for isdigit (renamed to avoid conflict)
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
    // Allow leading minus? Maybe. But *5 is error.
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
      // Unknown char? Should not happen with keypad
    }
  }

  // Ends with operator? 5+ is error
  if (lastIsOp)
    return 1;

  return 0;
}

// --- Helper Functions ---
void Calc_Reset(void) {
  g_bufferIndex = 0;
  memset(g_inputBuffer, 0, MAX_EXPR_LEN);
  lcdClearScreen();
  // Re-print prompt or cursor if needed, but clean slate is good.
  g_resetOnNextKey = 0;
  g_shiftActive = 0;
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

// Power Function (basic integer/float power)
double calc_pow(double base, double exp) {
  double res = 1.0;
  int i;
  // Handle integer positive exponents simply
  for (i = 0; i < (int)exp; i++)
    res *= base;
  return res;
  // Real implementation should use <math.h> pow() but that might bloat/fail
  // linking. For now, simple loop is safer for basic implementation.
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

  // Display Result
  // Move to next line? Or Clear?
  // Let's print "=" then result on next line or same line if space.
  // Given 20x4, let's just create a formatted string.

  char outStr[32];

  // Check if integer (cleaner output)
  if (result == (long)result) {
    sprintf(outStr, "= %ld", (long)result);
  } else {
    sprintf(outStr, "= %.3f", result);
  }

  // Store Result in Ans
  g_lastAns = result;

  // lcdGoto next line?
  // Let's just print it. The string has wrapping now.
  lcdWriteData(' ');    // Space before equals
  printDisplay(outStr); // Will wrap if needed

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

  // Map Keypad Chars to Operators
  // Variables declared at start of function (lines 189/190)
  // char displayChar = key; // REDECLARATION FIX
  // char bufferChar = key;  // REDECLARATION FIX

  // Reset defaults for this key
  displayChar = key;
  bufferChar = key;

  // Handle Shift Key Toggle ('D')

  if (key == 'D') {
    g_shiftActive = !g_shiftActive;
    // Visual feedback?
    // Maybe print 'S' at end of line 4? Or just trust user.
    // Let's print a small indicator if possible, or just ignore for now.
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
      // Shift+A = Ans (Recall)
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
          return;
        }
        return; // Buffer full
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

  // Add to buffer
  if (g_bufferIndex < MAX_EXPR_LEN - 1) {
    g_inputBuffer[g_bufferIndex++] = bufferChar;
    g_inputBuffer[g_bufferIndex] = '\0';

    // Echo to LCD
    lcdWriteData(displayChar);
  }
}
