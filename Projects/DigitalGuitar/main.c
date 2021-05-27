#include "stdlib.h"
#include "math.h"
#include "stm32f429xx.h"
#include "stm32f4xx_ll_bus.h"
#include "stm32f4xx_ll_gpio.h"
#include "stm32f4xx_ll_tim.h"
#include "stm32f4xx_ll_dac.h"
#include "stm32f4xx_ll_rng.h"
#include "sonate.h"

#define true            0x1
#define false           0x0

#define SAMPLE_RATE 8000
#define A_FREQ 440.0

//Частота дискретизации будет равна 8000 Гц
//Двойной буфер для воспроизведения.
//В начале программы должно сгенерироваться содержимое для двух буферов.
//Длину буфера возьмем равной 250 семлам
//Длительность звучания одного буфера будет равна 31.25 мс
//В одной секунде будет 32 целых буфера
//В случае конца воспроизведения буфер, в который идет запись значений должен заполниться нулями до его конца

typedef struct {
  int8_t* values;
  int size;
} signal;

static uint8_t buf[2][250];
static int activeBuf;
//static int played;
static int pcm_it = 0;
//static signal notesSig[16];
static int flags[16] = { 0 };
static int notes[16] = { 0 };
static int time[16] = { 0 };
static int t = 0;

float note2Hz(int note, float freq_of_main_tone) {
    note -= 69;
    //Коэффициент для вычисления частоты ноты. Это корень из 12-ой степени из 2.
    float magic_k=1.0594630943592952645618252949463;
    float freq = ((float)freq_of_main_tone)*pow(magic_k,note);
    return freq;
}

int isDelay(uint8_t b) {
    return (!(b >> 7));
}


float* generateNoise(int size) {
  //Сгенерировать шум от -1 до 1 в float
  float* noise = (float*)malloc(size*sizeof(float));
  for (int i = 0; i < size; ++i)
    noise[i] = 2*(((float)rand() / RAND_MAX) - 0.5);
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
    res.values = (int8_t*) malloc(res.size);
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

signal sineSignal(float frequency_dbl, float duration_dbl) {
  signal res;
  res.size = (int)(duration_dbl * SAMPLE_RATE);
  res.values = malloc(res.size);
  for (int i = 0; i < res.size; ++i) {
    res.values[i] = (int8_t)(127 * sin(2.*3.1416*frequency_dbl*i/SAMPLE_RATE));
  }
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


signal getSound(int note, int time, int ms) {
    //1. Найдем частоту ноты
    //2. Получим сигнал
    signal orig;
    if (((float)time / SAMPLE_RATE) > 0.3) {
        orig.values = NULL;
        orig.size = 0;
        return orig;
    }
    float duration = ((float)time / SAMPLE_RATE) + ((float)ms / 1000);
    if (duration > 0.3)
        duration = 0.3;
    float freq = note2Hz(note, A_FREQ);
    orig = getSignal(freq, duration);
    //3. Произведем обрезку сигнала
    int new_size = orig.size - time;
    signal new_signal;
    new_signal.size = new_size;
    new_signal.values = malloc(new_size);
    for (int i = time; i < orig.size; ++i) {
      new_signal.values[i - time] =  orig.values[i];
    }
    free(orig.values);
    return new_signal;
    
}

void genSound(int ms) {
    int t_end = t + ms * SAMPLE_RATE  / 1000;
    while (t_end >= t) {
        signal signal_byte[16];
        for(int i = 0; i < 16; ++i) {
          signal_byte[i].size = 0;
        }
        for (int i = 0; i < 16; ++i) {
            if (flags[i]) {
                //++time[i];
                signal_byte[i] = getSound(notes[i], time[i], ms);
                time[i] += ms * SAMPLE_RATE / 1000;
            }
        }
        t += ms * SAMPLE_RATE / 1000;
        signal res;
        res.size = 0;
        for (int i = 0; i < 16; ++i) {
          if (res.size < signal_byte[i].size) {
            res.size = signal_byte[i].size;
          }
        }
        float* s = malloc(res.size * sizeof(float));
        for (int i = 0; i < res.size; ++i)
          s[i] = 0;
        for (int i = 0; i < 16; ++i) {
          for (int j = 0; j < res.size; ++j) {
            if (j < signal_byte[i].size)
              s[j] += signal_byte[i].values[j];
          }
        }
        for (int i = 0; i < res.size; ++i) {
          s[i] /= 2;
        }
        res.values = malloc(res.size);
        for (int i = 0; i < res.size; ++i) {
          res.values[i] = (int8_t)s[i];
        }
        
        free(s);
        for (int i = 0; i < 16; ++i) {
           if (signal_byte[i].size != 0)
             free(signal_byte[i].values);
        }
        playSignal(res);
        res.size = 0;
        free(res.values);
        /*
        signal_byte /= 16;
        signal_dbl *= 127;
        int8_t s = signal_dbl;
        */
        //fwrite(&s, sizeof(short), 1, out);
        
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
  NVIC_EnableIRQ(TIM1_UP_TIM10_IRQn);
/*
  for (int i = 0; i < sizeof(buf[0]); ++i) {
    buf[0][i] = (int8_t)(127 * sin(2.*3.1416*440*i/SAMPLE_RATE) + 128); 
    buf[1][i] = (int8_t)(127 * sin(2.*3.1416*440*i/SAMPLE_RATE) + 128); 
  }
*/
  while(1) {
        for (int i = 0; i < sizeof(score); ++i) {
        if (isDelay(score[i])) {
            ++i;
            int ms = score[i] + 256 * (score[i - 1] % 128);
            genSound(ms);
        }
        else {
            if ( (score[i] >> 4) == 9 ) {
                flags[score[i] % 16] = true;
                time[score[i] % 16] = 0;
                notes[score[i] % 16] = score[i + 1];
                ++i;
            }
            else if ( (score[i] >> 4) == 8 ) {
                flags[score[i] % 16] = false; 
            }
            else if ( (score[i] == 0xF0) || (score[i] == 0xE0) ) {
                break;
            }
        }
    }
  }
}
