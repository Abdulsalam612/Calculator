#ifndef MENU_H
#define MENU_H

// Displays Main Menu and waits for selection
// Returns: 1 for Calculator, 2 for Tutorial
int Menu_Select(void);

// Runs the Tutorial (blocking until finished)
void Tutorial_Show(void);

#endif
