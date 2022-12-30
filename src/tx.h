
xSemaphoreHandle xTXDSP;
xSemaphoreHandle xTXIN;
xSemaphoreHandle xTXOUT;

void IRAM_ATTR tx_in(void * pvParameters){
  size_t readsize = 0;
  while(true){
    xSemaphoreTake(xTXIN, portMAX_DELAY);//ждем сигнала об окончании dsp-обработки предыдущего буфера
    if(current_mode==TX_MODE)i2s_read(I2S_NUM_0, &input_buffer, sizeof(input_buffer), &readsize, portMAX_DELAY);
     for (int i=0; i<NUM_SAMPLE_BUF; i++) { //копируем все принятые отсчеты в рабочий буфер
       workbuf_re[i] = (float)(input_buffer[i].re>>12);
       workbuf_im[i] = (float)(input_buffer[i].im>>12);
     }
     if(current_mode==TX_MODE)xSemaphoreGive(xTXDSP);//разрешаем dsp-обработку рабочего буфера
  }
}

void IRAM_ATTR tx_out(void * pvParameters){
  size_t readsize = 0;
  while(true){
    xSemaphoreTake(xTXOUT, portMAX_DELAY); //ждем сигнала об окончании dsp-обработки выходного буфера
    for (int i=0; i<NUM_SAMPLE_BUF; i++) { //переносим обработанный массив в выходной буфер с нормализацией в I2S-формат
         output_buffer[i].re = ((int)(workbuf_re[i]))<<12;
         output_buffer[i].im = ((int)(workbuf_im[i]))<<12;
    }
    if(current_mode==TX_MODE)i2s_write(I2S_NUM_0, &output_buffer, sizeof(output_buffer), &readsize, portMAX_DELAY );//вывод звука
  }
}

void IRAM_ATTR tx_dsp(void *pvParameters){
  while(true){
    xSemaphoreTake(xTXDSP, portMAX_DELAY);//ждем готовности рабочего буфера

    switch(rf_mode){//формируем квадратуры (гилберт+фнч)
    case USB:
      fir_f32(&fir_90, (float*)&workbuf_re, (float*)&workbuf_re, NUM_SAMPLE_BUF);//Hilbert90
      fir_f32(&fir_00, (float*)&workbuf_im, (float*)&workbuf_im, NUM_SAMPLE_BUF);//Hilbert00
      break;
    case LSB:
      fir_f32(&fir_00, (float*)&workbuf_re, (float*)&workbuf_re, NUM_SAMPLE_BUF);//Hilbert00
      fir_f32(&fir_90, (float*)&workbuf_im, (float*)&workbuf_im, NUM_SAMPLE_BUF);//Hilbert90
      break;
    }
    if(current_mode==TX_MODE){
        xSemaphoreGive(xTXIN);  //разрешаем прием следующей партии отсчетов 
        xSemaphoreGive(xTXOUT); //разрешаем выводить квадратуры в смеситель
    }
  }
}
