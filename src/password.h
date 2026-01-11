/*
 * File: password.h
 * Description: Public interface for the Password module.
 */

#ifndef PASSWORD_H

#define PASSWORD_H

// Initialize Password Module (Default PIN: 1234)
void Password_Init(void);

// Process Key Input for Password
void Password_Check(char key);

// Check if System is Unlocked
// Returns 1 if Unlocked, 0 if Locked
int Password_IsUnlocked(void);

// Lock the system (Optional, for re-locking)
void Password_Lock(void);

// Run blocking Change Password Routine
void Password_Change(void);

#endif
