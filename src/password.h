/*
 * File: password.h
 * Description: Public interface for the Password module.
 */

#ifndef PASSWORD_H

#define PASSWORD_H

// Initialize Password
void Password_Init(void);

// Process Key Input for Password
void Password_Check(char key);

// Check if System is Unlocked
// Returns 1 if Unlocked and then 0 if Locked
int Password_IsUnlocked(void);

// Lock the system
void Password_Lock(void);

// Change Password
void Password_Change(void);

#endif
