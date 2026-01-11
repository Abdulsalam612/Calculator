// main.c
// Minimal working version for TM4C123 / LaunchPad Port F LED+switch demo

#include "TExaS.h"
#include "tm4c123gh6pm.h"


// Constant declarations to access Port E registers using symbolic names
#define GPIO_PORTE_DATA_R   (*((volatile unsigned long *)0x400243FC))
#define GPIO_PORTE_DIR_R    (*((volatile unsigned long *)0x40024400))
#define GPIO_PORTE_AFSEL_R  (*((volatile unsigned long *)0x40024420))
#define GPIO_PORTE_PUR_R    (*((volatile unsigned long *)0x40024510))
#define GPIO_PORTE_DEN_R    (*((volatile unsigned long *)0x4002451C))
#define GPIO_PORTE_AMSEL_R  (*((volatile unsigned long *)0x40024528))
#define GPIO_PORTE_PCTL_R   (*((volatile unsigned long *)0x4002452C))
#define SYSCTL_RCGC2_R      (*((volatile unsigned long *)0x400FE108))


unsigned long In;   // PF4 input
unsigned long Out;  // PF3,PF2,PF1 output

// Prototypes
void PortE_Init(void);
void Delay(void);
// void EnableInterrupts(void); // not using it right now

void LED_Blue_on(void){
    // BLUE LED is PF2 -> bit 2 -> mask 0x04
    GPIO_PORTE_DATA_R |= 0x02;
}

int main(void){
    // TExaS_Init(SW_PIN_PF40,LED_PIN_PF321); // only needed if you're using the Valvano grader
    PortE_Init();

    while(1){
        In = GPIO_PORTE_DATA_R & 0x01;   // read PF4 (SW1) into In
        if(In == 0x00){
            // switch pressed (negative logic)
            LED_Blue_on();               // turn blue on
        } else {
            // switch not pressed
            GPIO_PORTE_DATA_R &= ~0x02;  // turn blue off (clear PF2)
        }

        Delay(); // ~0.1s delay
    }
}

// Initialize Port E: PE0 = input w/ pull-up, PE1..PE3 = outputs
void PortE_Init(void){ volatile unsigned long delay;
  SYSCTL_RCGC2_R |= 0x10;           // 1) activate clock for Port E
  delay = SYSCTL_RCGC2_R;           // allow time for clock to start
  GPIO_PORTE_AMSEL_R = 0x00;        // 2) disable analog on PE3-0
  GPIO_PORTE_PCTL_R  = 0x00000000;  // 3) PCTL GPIO on PE3-0
  GPIO_PORTE_DIR_R   = 0x0E;        // 4) PE3-PE1 outputs, PE0 input
  GPIO_PORTE_AFSEL_R = 0x00;        // 5) regular GPIO
  GPIO_PORTE_DEN_R   = 0x0F;        // 6) digital enable PE3-PE0
  GPIO_PORTE_PUR_R   = 0x01;        // 7) pull-up on PE0 (pressed = 0)
}
// Busy-wait ~0.1s delay.
// Matches the style in your template (same math).
void Delay(void){
    unsigned long volatile time;
    time = 727240*200/91;  // ~0.1s at ~80 MHz-ish lab assumptions
    while(time){
        time--;
    }
}
