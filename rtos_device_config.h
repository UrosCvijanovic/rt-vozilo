#pragma once
#include "stm32f405xx.h"
#define RTOS_DEVICE_LAST_IRQn FPU_IRQn

#define RTOS_KERNEL_TIM TIM2
#define RTOS_KERNEL_TIM_RCC_ENR RCC->APB1ENR
#define RTOS_KERNEL_TIM_RCC_EN_BIT  RCC_APB1ENR_TIM2EN  // MASKA, ne pozicija (kernel_timer.cpp:43)


#ifdef QEMU_SYSCLK_HZ
  // QEMU (netduinoplus2) - model tajmera ima fiksan ulazni takt 1 GHz
  #define RTOS_KERNEL_TIM_CLOCK_HZ   1000000000U
#else
  // Pravi F405 sa PLL-om na 168 MHz: APB1 = 42 MHz, tajmeri na APB1 dobijaju x2.
  // Bez podesenog PLL-a (HSI 16 MHz, APB1 bez delitelja) bilo bi 16000000U.
  #define RTOS_KERNEL_TIM_CLOCK_HZ   84000000U
#endif