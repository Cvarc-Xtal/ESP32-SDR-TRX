
#include "src/dsp_lib/xtensa_const_structs.h"

xtensa_cfft_instance_f32 cfft;
xtensa_cfft_instance_f32 rfft;

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
      cfft = xtensa_cfft_sR_f32_len1024;
      rfft = xtensa_cfft_sR_f32_len512;
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

    for (int i = 0 ; i < NUM_SAMPLE_BUF; i++) {
      input[i*2] = input[i*2] * wind[i];
      input[i*2+1] = input[i*2+1] * wind[i];
    }
    xtensa_cfft_f32(&rfft,input,0,1);
    for (int i=NUM_SAMPLE_BUF;i<NUM_SAMPLE_BUF*2;i++){
      input[i]=0;
    }
    xtensa_cmplx_mag_f32(input,fft,NUM_SAMPLE_BUF);// получение реальных значений спектральных составляющих
    float sum_fft = 0.0f;
    max_fft = 0.0f;
    float min_fft = 1000000.0f;
    int k = 0;
    for (int i = 0 ; i < NUM_SAMPLE_BUF/2; i++) {
        if(i>12){fft[i]*=0.01f;}else{fft[i]=0;}
        if (max_fft < fft[i])max_fft=fft[i];
        if (min_fft > fft[i])min_fft=fft[i];
        if(fft[i]>=700)fft[i]=700;
        //копирование магнитуд в отображаемый буфер,элементы которого постоянно уменьшаются
        if(fft[i]<=fft_inter[k]) fft[i]=fft_inter[k];
        if(fft[i]>fft_inter[k])fft_inter[k] = fft[i];
        //заполняем верхнюю строку массива для отображения "водопада"
        if(fill_fft)wp[wp_num[0]][k]=colors_w[sel_c((int)fft[i],FWW,CWW)];
        sum_fft = sum_fft+fft[i];
        k++;
    }
    avg_fft = (sum_fft-max_fft)/(NUM_SAMPLE_BUF/2);
    if(max_fft>600  && !dec_Ifgain) dec_Ifgain = true;
    if(avg_fft < 15 && !inc_Ifgain) inc_Ifgain = true;
    if(avg_fft > 20 && !dec_Ifgain) dec_Ifgain = true;   
}
