#include "coefficients.h"

FIR fir_rx;
FIR fir_90;
FIR fir_00;

static float delay_state_rx[NTAPS_RX];
static float delay_state_tx_90[NTAPS_TX];
static float delay_state_tx_00[NTAPS_TX];

void fir_init(FIR* fir,float* coeffs, float* delay, int N){ 
    fir->coeffs = coeffs;
    fir->delay = delay;
    fir->N = N;
    fir->pos = 0;
    for(int i=0;i<N;i++){
      fir->delay[i] = 0;
    }
}

void init_filters (uint8_t num_filter){
  uint8_t static old_filter = 100;
  if (num_filter == old_filter) return;
  for(int i=0;i<5;i++){acc_hpf[i]=acc_lpf[i]=0.0f;}
  switch (num_filter){
    case 0:indent=0;bandwidth=3000; fir_init(&fir_rx,lpf3000, delay_state_rx, NTAPS_RX);break;
    case 1:indent=0;bandwidth=2200; fir_init(&fir_rx,lpf2400, delay_state_rx, NTAPS_RX);break;
    case 2:indent=bandwidth=500;    fir_init(&fir_rx,bpf500,  delay_state_rx, NTAPS_RX);break;
    case 3:indent=0;bandwidth=6000; fir_init(&fir_rx,lpf6000, delay_state_rx, NTAPS_RX);break;
  }
  fir_init(&fir_90, h90, delay_state_tx_90, NTAPS_TX);
  fir_init(&fir_00, h00, delay_state_tx_00, NTAPS_TX);
  old_filter = num_filter;
  Serial.print("Init new FIR.");
}
