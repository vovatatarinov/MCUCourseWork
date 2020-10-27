#include "stm32f429xx.h"
#include "stm32f4xx_ll_bus.h"
#include "stm32f4xx_ll_gpio.h"
#include "stm32f4xx_ll_tim.h"


void TIM2_IRQHandler(void)
{
  // Blinking LED
  LL_TIM_ClearFlag_UPDATE(TIM2);
  LL_GPIO_TogglePin(GPIOG, LL_GPIO_PIN_13);
} 


int main()
{
  // Enable clocking for Timer 2 and GPIO Port G (for LED)
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOG);
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM2);
  
  // Setup GPIO as output for LED on GPIO PG13
  LL_GPIO_SetPinMode(GPIOG, LL_GPIO_PIN_13, LL_GPIO_MODE_OUTPUT);
  
  // Setup timer for generating interrupts every 1 s.
  LL_TIM_SetCounterMode(TIM2, LL_TIM_COUNTERMODE_UP);
  LL_TIM_SetPrescaler(TIM2, 1000 - 1);
  LL_TIM_SetAutoReload(TIM2, 8000 - 1);
  LL_TIM_EnableIT_UPDATE(TIM2);
  LL_TIM_EnableCounter(TIM2);

  // Enable interrupts on NVIC
  NVIC_EnableIRQ(TIM2_IRQn);
    
  while(1)
  {
  }
}
