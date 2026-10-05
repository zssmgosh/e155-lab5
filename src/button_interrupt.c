#include "main.h"
#include <stdio.h>
#include "stm32l432xx.h"

// declare global variables
volatile int direction; 
volatile int magCount;
volatile int pulseCount;
volatile float speed;
int A;
int B;

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
    GPIOA->PUPDR &= ~(0b11 << 2*gpioPinOffset(B_PIN)); // Set PA7 as pull-up (PUPD7 = 01)
    GPIOA->PUPDR &= ~(0b11 << 2*gpioPinOffset(A_PIN));
    GPIOA->PUPDR |= (0b01 << 2*gpioPinOffset(B_PIN)); // Set PA7 as pull-up (PUPD7 = 01)
    GPIOA->PUPDR |= (0b01 << 2*gpioPinOffset(A_PIN));

    // Initialize timer for printing
    RCC->APB2ENR |= (0b01 << 16); // TIM15EN
    initTIM(PRINT_TIM, 10000);

    // Enable SYSCFG clock domain in RCC
    RCC->APB2ENR |= (1 << 0); // SYSCFGEN
    // Configure EXTICR for the encoder interrupt
    // EXTI6 is bits 10:8 of EXTICR2 (EXTICR[1] in C). Port A is 0b000, so clearing the field selects PA6.
    SYSCFG->EXTICR[1] &= ~(0b111 << 8);
    // EXTI9 is bits 6:4 of EXTICR3 (EXTICR[2] in C). Port A is 0b000, so clearing the field selects PA9.
    SYSCFG->EXTICR[2] &= ~(0b111 << 4);

    // Configure interrupt for falling edge of GPIO pin for button
    EXTI->IMR1 |= (1 << gpioPinOffset(A_PIN));   // Configure mask bit
    EXTI->FTSR1 |= (1 << gpioPinOffset(A_PIN));  // Enable falling edge trigger
    EXTI->RTSR1 |= (1 << gpioPinOffset(A_PIN));  // Enable rising edge trigger
    
    EXTI->IMR1 |= (1 << gpioPinOffset(B_PIN));   // Configure mask bit
    EXTI->FTSR1 |= (1 << gpioPinOffset(B_PIN));  // Enable falling edge trigger
    EXTI->RTSR1 |= (1 << gpioPinOffset(B_PIN));  // Enable rising edge trigger

    NVIC->ISER[0] |= (1 << 23);                  // Turn on EXTI interrupt in NVIC_ISER (EXTI9_5 is IRQ 23)
    
    // Enable interrupts globally
    __enable_irq();
    __NVIC_EnableIRQ(EXTI9_5_IRQn);
    __NVIC_SetPriority(TIM1_BRK_TIM15_IRQn, 1);  // give print timer priority to get exact 1s intervals
    __NVIC_SetPriority(EXTI9_5_IRQn, 2);

    while(1){
        if (PRINT_TIM->CNT == 10000){
            speed = ((float)magCount)/(4*408.0f);
            printf("speed: %f!\n", speed);
            printf("direction: %d!\n", direction);
            PRINT_TIM->SR &= ~(0x1); // Clear UIF
            PRINT_TIM->CNT = 0; // reset counter
            magCount = 0;
        }
    }

}

// EXTI lines 5-9 share this handler, both PA6 and PA9 are in range
void EXTI9_5_IRQHandler(void){
    // Check which pin triggered our interrupt
    if (EXTI->PR1 & (1 << gpioPinOffset(A_PIN))){
        // Interrupt triggered on A, clear A and determine direction
        EXTI->PR1 |= (1 << gpioPinOffset(A_PIN));
        
        A = digitalRead(A_PIN);
        B = digitalRead(B_PIN);
        magCount++;

        // determine direction based on A and B values
        if (B == 0) {
            if (A == 1){
                direction = CW;
            } else {
                direction = CCW;
            }
        } else {
            if (A == 1){
                direction = CCW;
            } else {
                direction = CW;
            }
        }
    }

    if (EXTI->PR1 & (1 << gpioPinOffset(B_PIN))){
        // Interrupt triggered on B and determine direction
        EXTI->PR1 |= (1 << gpioPinOffset(B_PIN));
        A = digitalRead(A_PIN);
        B = digitalRead(B_PIN);
        magCount++;

        // determine direction based on A and B values
        if (B == 0) {
            if (A == 1){
                direction = CCW;
            } else {
                direction = CW;
            }
        } else {
            if (A == 1){
                direction = CW;
            } else {
                direction = CCW;
            }
        }
    }
}