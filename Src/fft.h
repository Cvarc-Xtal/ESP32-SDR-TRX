
#include "src/dsp_lib/xtensa_const_structs.h"

xtensa_rfft_fast_instance_f32 rfft;
xtensa_cfft_instance_f32 cfft;

float wind[NUM_SAMPLE_BUF];

void window_init(float *window, int len) 
{
    const float a0 = 0.42;
    const float a1 = 0.5;
    const float a2 = 0.08;

    float len_mult = 1/(float)(len-1);
    for (int i = 0; i < len; i++) {
        window[i] = a0 - a1 * cosf(i * 2 * M_PI * len_mult) + a2 * cosf(i * 4 * M_PI * len_mult);
    }

}


void fft_init(){
      xtensa_rfft_fast_init_f32(&rfft,NUM_SAMPLE_BUF);
      cfft = xtensa_cfft_sR_f32_len1024;
      window_init(wind,NUM_SAMPLE_BUF);//Blackman-window
      for(int i=WP_LINE;i>0;i--){wp_num[i-1]=i-1;} //нумеруем массив номеров строк "водопада"
}

int IRAM_ATTR sel_c(int val,int max,int mod){

  int md = max/mod;
  int i=0;
  if (val<=md)return i;
  for (i=0;i<mod;i++){
    if (i*md > val){return i;}
  }
  return i-1;
}


void IRAM_ATTR fft_for_display(float* input){
    
    static float input_tmp[NUM_SAMPLE_BUF];
    
    for (int i = 0 ; i < NUM_SAMPLE_BUF; i++) {
      input[i] = input[i] * wind[i];
    }
    xtensa_rfft_fast_f32(&rfft,input,input_tmp,0);         //БПФ
    xtensa_cmplx_mag_f32(input_tmp,input,NUM_SAMPLE_BUF/2);// получение реальных значений спектральных составляющих
    float sum_fft = 0.0f;
    float max_fft = 0.0f;
    float min_fft = 1000.0f;
    for (int i = 0 ; i < NUM_SAMPLE_BUF/2; i++) {
      input[i]*=0.001f;
      if (max_fft < input[i])max_fft=input[i];
      if (min_fft > input[i])min_fft=input[i];
      if(input[i]>=700)input[i]=700;
      //копирование магнитуд в отображаемый буфер,элементы которого постоянно уменьшаются
      if(input[i]<=fft_inter[i]) input[i]=fft_inter[i];
      if(input[i]>fft_inter[i])fft_inter[i] = input[i];
      //заполняем верхнюю строку массива для отображения "водопада"
      if(fill_fft)wp[wp_num[0]][i]=colors_w[sel_c((int)input[i],FWW,CWW)];
      sum_fft = sum_fft+input[i];
    }
    avg_fft = (sum_fft-max_fft)/(NUM_SAMPLE_BUF/2);
    if(max_fft>600  && !dec_Ifgain) dec_Ifgain = true;
    if(avg_fft < 15 && !inc_Ifgain) inc_Ifgain = true;
    if(avg_fft > 20 && !dec_Ifgain) dec_Ifgain = true;   
}
