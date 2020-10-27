#include "stm32f429xx.h"

uint8_t LedNumber = 0;


static void DelayMS(uint32_t TimeMS)
{
  const uint32_t TimeClocks = TimeMS * 16000 / 10;
  for (volatile uint32_t i = 0; i < TimeClocks; i++);
}


void EXTI0_IRQHandler(void)
{
  // When button is pressed PG14 led will blink
  // Otherwise - PG14 is blinking
  EXTI->PR |= EXTI_PR_PR0;
  LedNumber = LedNumber ? 0 : 1; 
}


int main()
{
  // Enable clocking for GPIO A and GPIO G
  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOGEN;
  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
  
  // Set GPIO PG13 and PG14 as outputs (for LEDs)
  GPIOG->MODER |= GPIO_MODER_MODER13_0;
  GPIOG->MODER |= GPIO_MODER_MODER14_0;
  
  // Setup EXTI for GPIO PA0 (button)
  EXTI->IMR |= EXTI_IMR_MR0;
  EXTI->FTSR |= EXTI_FTSR_TR0;
  EXTI->RTSR |= EXTI_RTSR_TR0;
  
  NVIC_EnableIRQ(EXTI0_IRQn);
  
  while(1)
  {
    // Blinking
    DelayMS(500);
    if (LedNumber == 0)
    {
      GPIOG->ODR |= GPIO_ODR_OD13;
    }
    else
    {
      GPIOG->ODR |= GPIO_ODR_OD14;
    }
    
    DelayMS(500);
    GPIOG->ODR &= ~(GPIO_ODR_OD13 | GPIO_ODR_OD14);
  }
}
