// main.h
// Josh Brake
// jbrake@hmc.edu
// 10/31/22

#ifndef MAIN_H
#define MAIN_H

#include "STM32L432KC.h"
#include <stm32l432xx.h>

///////////////////////////////////////////////////////////////////////////////
// Custom defines
///////////////////////////////////////////////////////////////////////////////

#define A_PIN PA6
#define B_PIN PA9
#define DELAY_TIM TIM2
#define PRINT_TIM TIM15
#define CW 0
#define CCW 1
#define MAXPULSE 408

#endif // MAIN_H