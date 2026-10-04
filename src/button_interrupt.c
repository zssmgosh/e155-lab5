#include "main.h"
#include <stdio.h>
#include "stm32l432xx.h"

// declare global variables
volatile int direction; 
volatile int magCount;
volatile int pulseCount;
volatile float speed;

int _write(int file, char *ptr, int len) {
  int i = 0;
  for (i = 0; i < len; i++) {
    ITM_SendChar((*ptr++));
  }
  return len;
}

int main(void) {
    // Enable encoder pins as input
    gpioEnable(GPIO_PORT_A);
    pinMode(A_PIN, GPIO_INPUT);
    pinMode(B_PIN, GPIO_INPUT);
    GPIOA->PUPDR |= (0b01 << 2*gpioPinOffset(B_PIN)); // Set PA7 as pull-up (PUPD7 = 01)
    GPIOA->PUPDR |= (0b01 << 2*gpioPinOffset(A_PIN));

    // Initialize timer
    RCC->APB1ENR1 |= (1 << 0); // TIM2EN
    initTIM(DELAY_TIM, 4000000000);
    RCC->APB2ENR |= (0b01 << 16); // TIM15EN
    initTIM(PRINT_TIM, 10000);

    // Enable SYSCFG clock domain in RCC
    RCC->APB2ENR |= (1 << 0); // SYSCFGEN
    // Configure EXTICR for the input button interrupt
    // EXTI7 is bits 14:12 of EXTICR2 (EXTICR[1] in C). Port A is 0b000, so clearing the field selects PA7.
    SYSCFG->EXTICR[1] &= ~(0b111 << 8);
    SYSCFG->EXTICR[2] &= ~(0b111 << 4);

    // Enable interrupts globally
    __enable_irq();

    // Configure interrupt for falling edge of GPIO pin for button
    EXTI->IMR1 |= (1 << gpioPinOffset(A_PIN));   // Configure mask bit
    EXTI->FTSR1 &= ~(1 << gpioPinOffset(A_PIN));  // Enable rising edge trigger
    EXTI->RTSR1 |= (1 << gpioPinOffset(A_PIN)); // Disable falling edge trigger
    NVIC->ISER[0] |= (1 << 23);                  // Turn on EXTI interrupt in NVIC_ISER (EXTI9_5 is IRQ 23)
    
    while(1){
        pulseCount = resetTIMCNT(DELAY_TIM, magCount, MAXPULSE);
        if(magCount >= MAXPULSE){
            magCount = 0;
        }
        speed = 10000.0f/((float)pulseCount); // CNT/f_psc = time for one revolution, speed = f_psc/CNT
        //delay_millis(DELAY_TIM, 200);
        if (PRINT_TIM->CNT == 10000){
            printf("pulse count: %d\n", pulseCount);
            printf("magcount %d ", magCount);
            printf("speed: %f!\n", speed);
            printf("direction: %d!\n", direction);
            PRINT_TIM->SR &= ~(0x1); // Clear UIF
            PRINT_TIM->CNT = 0;
        }
    }

}

// EXTI lines 5-9 share this handler
void EXTI9_5_IRQHandler(void){
    int A = 0;
    int B = 0;
    // Check that the button was what triggered our interrupt
    if (EXTI->PR1 & (1 << gpioPinOffset(A_PIN))){
        // If so, clear the interrupt (NB: Write 1 to reset.)
        EXTI->PR1 = (1 << gpioPinOffset(A_PIN));

        //A = digitalRead(A_PIN);
        B = digitalRead(B_PIN);

        if (B == 0) {
            direction = CW;
        } else {
            direction = CCW;
        }

        magCount++;
        
    }
}