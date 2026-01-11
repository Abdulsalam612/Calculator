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
#define SYSCTL_RCC_XTAL_25MHZ                                                  \
  0x00000540 // XTAL Value for 16MHz Crystal (wait, usually Launchpads are
             // 16MHz)
// Actually, standard TExaS/Launchpad PLL instructions often use 16MHz
// crystal setting.// 0x15 in XTAL field for 16MHz. Let's use the standard
// robust Valvano-style PLL init logic.

void SysPLL_Init(void) {
  // 0) Use RCC2 because it provides more options (like 400MHz PLL)
  SYSCTL_RCC2_R |= 0x80000000; // USERCC2

  // 1) Bypass PLL while initializing
  SYSCTL_RCC2_R |= 0x00000800; // BYPASS2, PLL bypass

  // 2) Select the crystal value and oscillator source
  SYSCTL_RCC_R = (SYSCTL_RCC_R & ~0x000007C0) // clear XTAL field
                 + 0x00000540;                // configure for 16 MHz crystal
  SYSCTL_RCC2_R &= ~0x00000070; // configure for main oscillator source

  // 3) Activate PLL by clearing PWRDN
  SYSCTL_RCC2_R &= ~0x00002000;

  // 4) Set the desired system divider
  // The PLL operates at 400 MHz (with USERCC2).
  // We want 80 MHz. 400 MHz / 5 = 80 MHz.
  // SYSDIV2 = 5 - 1 = 4.
  SYSCTL_RCC2_R |= 0x40000000; // use 400 MHz PLL
  SYSCTL_RCC2_R =
      (SYSCTL_RCC2_R & ~0x1FC00000) // clear system clock divider field
      + (4 << 22);                  // configure for 80 MHz clock

  // 5) Wait for the PLL to lock by checking PLLLRIS
  while ((SYSCTL_RIS_R & 0x00000040) == 0) {
  };

  // 6) Enable use of PLL by clearing BYPASS
  SYSCTL_RCC2_R &= ~0x00000800;
}
