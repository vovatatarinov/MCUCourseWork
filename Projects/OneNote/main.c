#include "stdlib.h"
#include "stdint.h"
#include "math.h"
#include "stm32f429xx.h"
#include "stm32f4xx_ll_bus.h"
#include "stm32f4xx_ll_gpio.h"
#include "stm32f4xx_ll_tim.h"
#include "stm32f4xx_ll_dac.h"
#include "stm32f4xx_ll_rng.h"
//#include "shurik.h"

#define SAMPLE_RATE 8000

//Частота дискретизации будет равна 8000 Гц
//Двойной буфер для воспроизведения.
//В начале программы должно сгенерироваться содержимое для двух буферов.
//Длину буфера возьмем равной 250 семлам
//Длительность звучания одного буфера будет равна 31.25 мс
//В одной секунде будет 32 целых буфера
//В случае конца воспроизведения буфер, в который идет запись значений должен заполниться нулями до его конца

typedef struct {
  int8_t* values;
  //int* values;
  int size;
} signal;

static uint8_t buf[2][250];
static int activeBuf;
//static int played;
static int pcm_it = 0;
//static signal notesSig[16];

float* generateNoise(int size) {
  //Сгенерировать шум от -1 до 1 в float
  float* noise = (float*)malloc(size*sizeof(float));
  for (int i = 0; i < size; ++i)
    noise[i] =  2*(((float)rand() / RAND_MAX) - 0.5);
  return noise;
  
}

void normalizeSignal(signal input) {
    if (input.size == 0)
        return;
    int8_t max;
    max = labs(input.values[0]);
    for (int i = 0; i < input.size; ++i) {
      if (labs(input.values[i]) > max)
          max = labs(input.values[i]);
    }
    if (max != 0) {
      float k = 127. / (float)max;
      for (int i = 0; i < input.size; ++i) {
        input.values[i] = (int8_t)((float)input.values[i] * k);
      }
    }
}

signal getSignal(float frequency_dbl, float duration_dbl) {
    int noiseSize = (int)((float)SAMPLE_RATE / frequency_dbl);
    float* noise = generateNoise(noiseSize);
    signal res;
    res.size = (int)(duration_dbl * SAMPLE_RATE);
    float* s = (float*) malloc(res.size * sizeof(float));
    //res.values = (int8_t*) malloc(res.size);
    res.values = (int8_t*) malloc(res.size*sizeof(int8_t));
    for (int i = 0; i < res.size; ++i)
      res.values[i] = 0;
    for (int i = 0; ( (i < noiseSize) || (i < res.size) ); ++i)
      s[i] = noise[i];
    for (int i = noiseSize; i < res.size; ++i)
      //Коэффициент подбирается
      s[i] = 0.996 * (float)((s[i - noiseSize + 1] + s[i - noiseSize]) / 2.);
    free(noise);
    for (int i = 0; i < res.size; ++i)
      res.values[i] = (int8_t)(s[i] * 127);
    //normalizeSignal(res);
    free(s);
    return res;
}

void playSignal(signal input) {
  for (int i = 0; i <= input.size / 250; ++i) {
    int nonActiveBuf = 0;
    if (activeBuf == 0)
        nonActiveBuf = 1;
    if ( (pcm_it >= 0) && (pcm_it < 125) ){
    for (int j = 0; j < 250; ++j) {
      int k = i * 250 + j;
      if (k < input.size)
        buf[nonActiveBuf][j] = (uint8_t)(((long)input.values[k] + 128));
        //buf[nonActiveBuf][j] = input.values[k];
      else
        buf[nonActiveBuf][j] = 128;
      //played = 0;
    }
     while(pcm_it != 0);
    }
    
  }

}

void TIM1_UP_TIM10_IRQHandler() {
  LL_TIM_ClearFlag_UPDATE(TIM1);
  LL_DAC_ConvertData12RightAligned(DAC1, LL_DAC_CHANNEL_1, (long)buf[activeBuf][pcm_it] * 4096 / 256);
  ++pcm_it;
  if (pcm_it >= 250) {
    //played = 1;
    pcm_it = 0;
    if (activeBuf == 0)
      activeBuf = 1;
    else
      activeBuf = 0;
  }
}
static signal Anote;
static signal Bnote;
static signal res;
static int8_t a = -8;
static int8_t b = 3;
static int8_t c;
int main() {

  //played = 0;
  activeBuf = 0;
  for (int i = 0; i < 2; ++i)
    for (int j = 0; j < 250; ++j)
      buf[i][j] = 0;
  
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
  //Найдем зерно для srand
  //LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_RNG); //RNG settings
  //LL_RNG_Enable(RNG);
  //while(LL_RNG_IsActiveFlag_DRDY(RNG)==0);
  //seed = LL_RNG_ReadRandData32(RNG);
  srand(0x55aaff00);
  c = a / b;
  Anote = getSignal(330, 1);
  Bnote = getSignal(440, 1);
  
  res.size = 8000;
  int8_t values[8000];
  res.values = values;
  for (int i = 0; i < 8000; ++i) {
    res.values[i] = ((float)Anote.values[i] / 3);// + (Bnote.values[i] / 1.2);
  }
  free(Anote.values);
  free(Bnote.values);
  NVIC_EnableIRQ(TIM1_UP_TIM10_IRQn);
  while(1) {
    playSignal(res);
  }
}
