// STM32F401RE_TIM.c
// TIM functions

#include "STM32L432KC_TIM.h"
#include "STM32L432KC_RCC.h"

void initTIM(TIM_TypeDef * TIMx, uint32_t ms){
  // Set prescaler to give 1 ms time base
  uint32_t psc_div = (uint32_t) ((SystemCoreClock/1e4));

  // Set prescaler division factor
  TIMx->PSC = (psc_div - 1);
  // Set timer max count
  TIMx->ARR = ms;
  // Generate an update event to update prescaler value
  TIMx->EGR |= 1;
  // Enable counter
  TIMx->CR1 |= 1; // Set CEN = 1
  TIMx->SR &= ~(0x1); // Clear UIF
  TIMx->CNT = 0;      // Reset count
}

//void delay_millis(TIM_TypeDef * TIMx, uint32_t ms){
//  TIMx->ARR = ms;// Set timer max count
//  TIMx->EGR |= 1;     // Force update
//  TIMx->SR &= ~(0x1); // Clear UIF
//  TIMx->CNT = 0;      // Reset count

//  while(!(TIMx->SR & 1)); // Wait for UIF to go high
//}

int resetTIMCNT(TIM_TypeDef * TIMx, int magCount, int maxCount){
  int pulseCount;
  if (magCount >= 408) {
    pulseCount = TIMx->CNT;
    TIMx->SR &= ~(0x1); // Clear UIF
    TIMx->CNT = 0;
  }
  return pulseCount;
}