/*
 * File: Flash.h
 * Description: Public interface for Flash memory operations.
 */

#ifndef FLASH_H

#define FLASH_H

#include <stdint.h>

// Address to store the Password
#define FLASH_PASSWORD_ADDR 0x00020000

// Initialize Flash 
void Flash_Init(void);

void Flash_Erase(uint32_t addr);

// Write a 32-bit word to the given address
// Returns 0 on success, non-zero on error
int Flash_Write(uint32_t addr, uint32_t data);

// Read a 32-bit word from the given address
uint32_t Flash_Read(uint32_t addr);

#endif
