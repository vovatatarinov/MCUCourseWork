#include "stm32f429xx.h"
#include "stm32f4xx_ll_bus.h"
#include "stm32f4xx_ll_gpio.h"
#include "stm32f4xx_ll_tim.h"
#include "stm32f4xx_ll_dac.h"
#include "test.u8.8000Hz.h"

static int pcm_it = 0;

void TIM1_UP_TIM10_IRQHandler() {
  LL_TIM_ClearFlag_UPDATE(TIM1);
  LL_DAC_ConvertData12RightAligned(DAC1, LL_DAC_CHANNEL_1, (long)pcm_data[pcm_it] * 4096 / 256);
  ++pcm_it;
  if (pcm_it >= sizeof(pcm_data))
      pcm_it = 0;
}

int main()
{
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_DAC1);
  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_TIM1);
  LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_4,  LL_GPIO_MODE_ANALOG);
  LL_TIM_SetPrescaler(TIM1, 0);
  LL_TIM_SetCounterMode(TIM1, LL_TIM_COUNTERMODE_UP);
  LL_TIM_SetAutoReload(TIM1, 1999);
  LL_TIM_EnableUpdateEvent(TIM1);
  LL_TIM_EnableIT_UPDATE(TIM1);
  LL_TIM_EnableCounter(TIM1);
  LL_DAC_Enable(DAC1, LL_DAC_CHANNEL_1);
  NVIC_EnableIRQ(TIM1_UP_TIM10_IRQn);
  while(1)
  {
  }
}
