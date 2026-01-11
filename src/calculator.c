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

// Helper for isdigit
int is_digit(char c) { return (c >= '0' && c <= '9'); }

#define MAX_EXPR_LEN 64
#define MAX_STACK 32

// State Management
static char g_inputBuffer[MAX_EXPR_LEN];
static int g_bufferIndex = 0;
static int g_resetOnNextKey = 0;

// Stacks for Evaluation
static double valStack[MAX_STACK];
static int valTop = -1;
static char opStack[MAX_STACK];
static int opTop = -1;

// --- Helper Functions ---
void Calc_Reset(void) {
  g_bufferIndex = 0;
  memset(g_inputBuffer, 0, MAX_EXPR_LEN);
  lcdClearScreen();
  // Re-print prompt or cursor if needed, but clean slate is good.
  g_resetOnNextKey = 0;
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
  return 0;
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
  default:
    return 0.0;
  }
}

// Evaluate the buffered string
void Calc_Evaluate(void) {
  int i;
  valTop = -1;
  opTop = -1;

  // Simple Parser
  for (i = 0; i < g_bufferIndex; i++) {
    // Skip whitespace
    if (g_inputBuffer[i] == ' ')
      continue;

    // If Digit or Decimal point, parse number
    if (is_digit(g_inputBuffer[i]) || g_inputBuffer[i] == '.') {
      char numStr[32];
      int k = 0;
      // Capture full number
      while (i < g_bufferIndex &&
             (is_digit(g_inputBuffer[i]) || g_inputBuffer[i] == '.')) {
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
    snprintf(outStr, 32, "= %ld", (long)result);
  } else {
    snprintf(outStr, 32, "= %.3f", result);
  }

  // lcdGoto next line?
  // Let's just print it. The string has wrapping now.
  lcdWriteData(' ');    // Space before equals
  printDisplay(outStr); // Will wrap if needed

  g_resetOnNextKey = 1; // Flag to clear on next input
}

// --- Public Interface ---
void Calc_Init(void) { Calc_Reset(); }

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
    break; // Multiplication
  case 'D':
    bufferChar = '/';
    displayChar = '/';
    break; // Division
  // case '*': removed (now backspace)
  default:
    break;
  }

  // Add to buffer
  if (g_bufferIndex < MAX_EXPR_LEN - 1) {
    g_inputBuffer[g_bufferIndex++] = bufferChar;
    g_inputBuffer[g_bufferIndex] = '\0';

    // Echo to LCD
    lcdWriteData(displayChar);
  }
}
