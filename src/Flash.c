/*
 * File: Flash.c
 * Description: Low-level driver for the TM4C123GH6PM internal Flash memory.
 *              Supports Erase and Write operations.
 */

#include "Flash.h"

// TM4C123 Flash Registers
#define FLASH_FMA_R (*((volatile uint32_t *)0x400FD000))
#define FLASH_FMD_R (*((volatile uint32_t *)0x400FD004))
#define FLASH_FMC_R (*((volatile uint32_t *)0x400FD008))
#define FLASH_FMC_WRKEY 0xA4420000
#define FLASH_FMC_WRITE 0x00000001
#define FLASH_FMC_ERASE 0x00000002

void Flash_Init(void) {}

void Flash_Erase(uint32_t addr) {
  FLASH_FMA_R = addr;
  FLASH_FMC_R = FLASH_FMC_WRKEY | FLASH_FMC_ERASE;

  // Poll until ERASE bit is cleared
  while (FLASH_FMC_R & FLASH_FMC_ERASE)
    ;
}

int Flash_Write(uint32_t addr, uint32_t data) {
  FLASH_FMA_R = addr;
  FLASH_FMD_R = data;
  FLASH_FMC_R = FLASH_FMC_WRKEY | FLASH_FMC_WRITE;

  // Poll until WRITE bit is cleared
  while (FLASH_FMC_R & FLASH_FMC_WRITE)
    ;

  return 0;
}

uint32_t Flash_Read(uint32_t addr) {
  return *((volatile uint32_t *)(uintptr_t)addr);
}
