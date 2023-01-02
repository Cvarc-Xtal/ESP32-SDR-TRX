

void i2sinit(){
   
   i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_TX),
    .sample_rate = (int) I2S_SAMPLE_RATE,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
    .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
    .communication_format = I2S_COMM_FORMAT_STAND_I2S,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 4,
    .dma_buf_len = 256,
    .use_apll = false,
    .tx_desc_auto_clear = false,
    .mclk_multiple = I2S_MCLK_MULTIPLE_384 //master-clock on GPIO0
   };
   i2s_pin_config_t pin_config = {
     .mck_io_num =   I2S_MCLK,
     .bck_io_num =   I2S_BCK,
     .ws_io_num =    I2S_WS,
     .data_out_num = I2S_DOUT,
     .data_in_num =  I2S_DIN                                                       
   };
   i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
   i2s_set_clk(I2S_NUM_0,I2S_SAMPLE_RATE,I2S_BITS_PER_SAMPLE_32BIT,I2S_CHANNEL_STEREO);
   i2s_set_pin(I2S_NUM_0, &pin_config);
}

//АЦП для кнопок.Резистивные делители (5/6 резисторов по 1 кОм и 4/5 кнопок на канал)
//номиналы не критичны - имеется программная калибровка в SETUP
void adc_init(){ 
 adc1_config_width(ADC_WIDTH_9Bit);
 adc1_config_channel_atten(LEFT_ADC, ADC_ATTEN_11db);
 adc1_config_channel_atten(RIGHT_ADC, ADC_ATTEN_11db);
}

PCF8574 pcf(0x27);

void pcf_init(){
 pcf.pinMode(0, OUTPUT);
 pcf.pinMode(1, OUTPUT);
 pcf.pinMode(2, OUTPUT);
 pcf.pinMode(3, OUTPUT);
 pcf.pinMode(4, OUTPUT);
 pcf.digitalWrite(0,LOW);
 pcf.digitalWrite(1,LOW);
 pcf.digitalWrite(2,LOW);
 pcf.digitalWrite(3,LOW);
 pcf.digitalWrite(4,LOW);
}

void buf_init(){
  for(int i=0;i<NUM_SAMPLE_BUF;i++){
    input_buffer[i].re = input_buffer[i].im = output_buffer[i].re = output_buffer[i].im = 0;
    workbuf_re[i] = workbuf_im[i] = 0.0f;workbuf_in[i].re = workbuf_in[i].im = 0.0f;
  }
}

void start_ok(){

 tft.clear(0);
 gfx.drawRGBBitmap(70,100,myBitmap,344,72);
 tft.setFont(CodePage437_8x14);
 tft.setCursor(10,10);tft.setTextColor(WHITE,BLACK);tft.print(VERSION);
 tft.setCursor(240,230);tft.setTextColor(WHITE,BLACK);tft.print("Click encoder to SETUP");
 tft.show();
 
 for(int x = 70;x<300;x++){ //заставка
    tft.fillRect(80,170,x,5,colors[5]);
    if(digitalRead(ROTARY_ENCODER_BUTTON_PIN)==LOW)
        {current_mode=SETUP_MODE;readConfig();redraw=true;speak_out = false;break;}
    delay(10);
  }
  speak_out = true;
 tft.clear(0);
}
