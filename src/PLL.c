/*
 * File: PLL.c
 * Description: Phase Locked Loop (PLL) configuration to set system clock to 80
 * MHz.
 */

#include "PLL.h"

// Register Definitions for RCC and RCC2
#define SYSCTL_RIS_R (*((volatile unsigned long *)0x400FE050))
#define SYSCTL_RCC_R (*((volatile unsigned long *)0x400FE060))
#define SYSCTL_RCC2_R (*((volatile unsigned long *)0x400FE070))

// Constants
#define SYSCTL_RCC_XTAL_25MHZ 0x00000540 // XTAL Value for 16MHz Crystal

void SysPLL_Init(void) {
  SYSCTL_RCC2_R |= 0x80000000;

  //Bypass PLL while initializing
  SYSCTL_RCC2_R |= 0x00000800;

  // Select the crystal value and oscillator source
  SYSCTL_RCC_R = (SYSCTL_RCC_R & ~0x000007C0)
                 + 0x00000540;
  SYSCTL_RCC2_R &= ~0x00000070;

  // Activate PLL by clearing PWRDN
  SYSCTL_RCC2_R &= ~0x00002000;

  // Set the desired system divider

  SYSCTL_RCC2_R |= 0x40000000; // use 400 MHz PLL
  SYSCTL_RCC2_R =
      (SYSCTL_RCC2_R & ~0x1FC00000)
      + (4 << 22);                  // configure for 80 MHz clock

  //Wait for the PLL to lock by checking PLLLRIS
  while ((SYSCTL_RIS_R & 0x00000040) == 0) {
  };

  SYSCTL_RCC2_R &= ~0x00000800;
}
