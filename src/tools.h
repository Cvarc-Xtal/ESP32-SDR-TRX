

AiEsp32RotaryEncoder rotaryEncoder = AiEsp32RotaryEncoder(ROTARY_ENCODER_A_PIN, ROTARY_ENCODER_B_PIN, ROTARY_ENCODER_BUTTON_PIN, ROTARY_ENCODER_VCC_PIN, ROTARY_ENCODER_STEPS);
Si5351 si5351;

void action(){ //обработка нажатий на кнопки
  if(current_mode==RX_MODE){ //на экране приема
      if (lkey == 1){numband++;if(numband>N_BANDS-1)numband=0;show_band_time = SHOW_BAND;redraw=true;}
      if (lkey == 2){If_gain++; if(If_gain>31){If_gain=31;}wm8731_set_line_in_volume(If_gain);}
      if (lkey == 3){If_gain--; if(If_gain<1) {If_gain=0; }wm8731_set_line_in_volume(If_gain);}
      if (lkey == 4){step_freq=step_freq*10;key_rotary=false;if (step_freq > 10000)step_freq=10;}
      if (lkey == 5){}
      if (rkey == 1){num_filter++;if(num_filter >3)num_filter = 0;}//
      if (rkey == 2){rf_mode++;if(rf_mode>AM)rf_mode=LSB;} // lsb/usb/am
      if (rkey == 3){filter_gain++;if(filter_gain>2)filter_gain=0;}
      if (rkey == 4){tuning = !tuning;if(tuning)pos_fft=PCH;redraw_freq = true;}
      if (rkey == 5){flag_write_parameters = true;}//кнопка энкодера
      lkey = 0; rkey = 0;
  }
  if(current_mode==SETUP_MODE){ //на экране настройки кнопок
    if (rkey == 5 && !flag_exit_setup && n_button<8){flag_write_config=true;}
    if ((rkey == 5) && flag_exit_setup){current_mode=RX_MODE;xSemaphoreGive(xRXIN);redraw=true;flag_exit_setup=false;speak_out = true;}
    lkey=0;rkey=0;
  }
}

void scroll_wp(){ //сдвиг массива строк "водопада"

      fill_fft=false;//запретить заполнение fft-буфера на время сдвига "водопада" 
      uint8_t tmp = wp_num[WP_LINE-1];
      for (int i=WP_LINE-1;i>0;i--) {wp_num[i] = wp_num[i-1];}
      wp_num[0]=tmp;
      fill_fft = true;//разрешаем запонение буфера fft
}

void ag(){

      if(dec_Ifgain){
              If_gain--;
              if(If_gain < 1){If_gain = 1;return;}
              wm8731_set_line_in_volume(If_gain);
              dec_Ifgain=false;
              }
      if(inc_Ifgain){
              If_gain++;
              if(If_gain>31){If_gain=31;return;}
              wm8731_set_line_in_volume(If_gain);
              inc_Ifgain=false;
         }
}
void write_parameters(){
  /*номер     параметр
   * 8        номер диапазона
   * 9        частота настройки
   * 10       шаг перестройки
   * 11       номер основного фильтра
  */
  if(flag_write_parameters){
    EEPROM.writeUInt(8*sizeof(uint32_t),(uint32_t)numband);
    EEPROM.writeUInt(9*sizeof(uint32_t),(uint32_t)freq);
    EEPROM.writeUInt(10*sizeof(uint32_t),(uint32_t)step_freq);
    EEPROM.writeUInt(11*sizeof(uint32_t),(uint32_t)num_filter);
    EEPROM.commit();
    flag_write_parameters = false;
   
    Serial.println("Saved parameters");
  }
}
void time1(){
  int static ms1 = 0;
   if((cur_ms < ms1) || ((cur_ms - ms1) > 1000 )){//1sec
      ms1 = cur_ms;
      if(show_band_time==1)show_band_time=0;
      if(show_band_time==0)redraw=true;
      show_band_time--;
      if(show_band_time<0)show_band_time=-1;
      write_parameters();
      //ag(); //АРУ по входу кодека
      //Serial.print("xRX :");Serial.println(r_rx);
      //Serial.print("xDSP:");Serial.println(r_dsp);
      //Serial.print("xOUT:");Serial.println(cycl_result_out);
      //Serial.print("xDRAW:");Serial.println(cycl_result_draw);
      //Serial.print("xCNTR:");Serial.println(cycl_result_cntr);

  }
}
//опрос и обработка кнопок
void time_01(){
  int static ms01 = 0;
   if((cur_ms < ms01) || ((cur_ms - ms01) > 50 )){ //50mses
       ms01 = cur_ms;
       if(current_mode!=SETUP_MODE){l_knopka();r_knopka();}//опрос кнопок
       if (l_release){l_release = false;l_press=false;action();}//если отпущена л.кнопка вызов обработчика
       if (r_release){r_release = false;r_press=false;action();}//если отпущена пр.кнопка вызов обработчика
   }
}

void time_02(){
  int static ms02 = 0;
  if((cur_ms < ms02) || ((cur_ms - ms02) > 50 )){
       ms02 = cur_ms;
       encoderDelta = rotaryEncoder.encoderChanged();
       if(current_mode==RX_MODE){ //реакция на поворот энкодера на 0 экране
          if ((encoderDelta > 0) && (tuning)) {freq = freq+step_freq;if(freq > 39999999)freq=39999999;}//
          if ((encoderDelta < 0) && (tuning)) {freq = freq-step_freq;if(freq < 100000)freq=100000;}
          if ((encoderDelta > 0) && (!tuning)){pos_fft+=2;if(pos_fft > 480/m_screen)pos_fft=480/m_screen;redraw_freq = true;}
          if ((encoderDelta < 0) && (!tuning)){pos_fft-=2;if(pos_fft < 4)pos_fft=4;redraw_freq = true;}    
       }
       if(current_mode==SETUP_MODE){//реакция на поворот энкодера на 2 экране
        if (encoderDelta > 0){n_button++;if(n_button>8)n_button=8;redraw=true;}
        if (encoderDelta < 0){n_button--;if(n_button<0)n_button=0;redraw=true;}
       }
  }
}

void time_05(){
  int static ms05 = 0;
   if((cur_ms < ms05) || ((cur_ms - ms05) > 50 )){//таймер - скорость "водопада"
       ms05 = cur_ms;
       scroll_wp();//прокручиваем массив fft-строк
       old_smeter--;//декремент показателя s-метра
       if(old_smeter<=1)old_smeter=1;
       agc_koeff = (120/old_smeter);if(agc_koeff>15)agc_koeff=15; //АРУ
   }
}

void time_001(){
  int static ms001 = 0;
   if((cur_ms < ms001) || ((cur_ms - ms001) > 10 )){//10msec таймер - скорость спада пиков спектра
       ms001 = cur_ms;
       for (int i=0;i<NUM_SAMPLE_BUF;i++){
        if (fft_inter[i]>300)fft_inter[i]-=30;
        if (fft_inter[i]>100)fft_inter[i]-=10;
        if (fft_inter[i]>50)fft_inter[i]-=5;
        if (fft_inter[i]<=50)fft_inter[i]--;
        }
       }
}

void rotary_onButtonClick()
{
  static unsigned long lastTimePressed = 0;
  if (millis() - lastTimePressed < 300)
  {
    key_rotary = false; return;
  }
  lastTimePressed = millis();
  rkey=5;r_release=true;

}

void bpf_set_band(int set_band){

 pcf.digitalWrite(0,LOW);
 pcf.digitalWrite(1,LOW);
 pcf.digitalWrite(2,LOW);
 pcf.digitalWrite(3,LOW);
 pcf.digitalWrite(4,LOW);
 pcf.digitalWrite(set_band,HIGH);
 
}

void changeFrequency( int currentFrequency )
{
  static int lastFreq = -1;
  static int last_band = -1;
  int band=0;
  if (lastFreq == currentFrequency)return;
  if(currentFrequency>15000000UL) band=4;
  else if(currentFrequency>8000000UL) band=3;
  else if(currentFrequency>4000000UL) band=2;
  else if(currentFrequency>2000000UL) band=1;
  else band=0;
  if(band != last_band){bpf_set_band(band);last_band = band;}
  if (si) {si5351.set_freq(currentFrequency * SI5351_FREQ_MULT*4,SI5351_CLK2);
           si5351.output_enable(SI5351_CLK2, 1);}
  lastFreq = currentFrequency;redraw_freq = true;
}

void change_band(){
  static int old_band=100;
  static bool first = true;
  if (old_band==numband)return;
  if(!first)freq = bands[numband].freq;
  rf_mode  = bands[numband].mode;
  if(!first)num_filter = bands[numband].filter; 
  old_band=numband;first = false;
}

void check_ptt(){
    static bool old_tx = true;
    if (digitalRead(PTT) == LOW && !old_tx){ //доработать сдвиг частоты SI5351 при передаче
      current_mode = TX_MODE;
      old_tx = true;speak_out = false;
      buf_init();
      xSemaphoreGive(xTXIN);
      }
    if (digitalRead(PTT) == HIGH && old_tx){
      current_mode = RX_MODE;
      old_tx = false;speak_out = true;
      buf_init();
      xSemaphoreGive(xRXIN);
      }
}
void control(){
    cur_ms = millis();
    if (rotaryEncoder.isEncoderButtonClicked()){rotary_onButtonClick();}
    if(current_mode!=SETUP_MODE)check_ptt();
    if(current_mode==RX_MODE){
      change_band();
      changeFrequency(freq);
      time1();
      time_05();
      time_001();
    }
    time_01();
    time_02();
}

void readConfig(){
  /*номер     параметр
   * 0        значение ацп для лев.кнопки №1
   * 1        значение ацп для лев.кнопки №2
   * 2        значение ацп для лев.кнопки №3
   * 3        значение ацп для лев.кнопки №4
   * 4        значение ацп для прав.кнопки №1
   * 5        значение ацп для прав.кнопки №2
   * 6        значение ацп для прав.кнопки №3
   * 7        значение ацп для прав.кнопки №4
   * 8        номер диапазона
   * 9        частота настройки
   * 10       шаг перестройки
   * 11       номер основного фильтра
  */
  int i = 0;
  for(i=0;i<8;i++){
    value_button[i]=EEPROM.readUInt(i*sizeof(uint32_t));
    if(value_button[i]>511)value_button[i]=999;
  }
  numband = EEPROM.readUInt(i*sizeof(uint32_t));if(numband>N_BANDS-1||numband<0)numband=0;i++;
  freq = EEPROM.readUInt(i*sizeof(uint32_t));if(freq>30000000||freq<100000)freq = bands[numband].freq;i++;
  step_freq = EEPROM.readUInt(i*sizeof(uint32_t));if(step_freq>10000||step_freq<10)step_freq=100;i++;
  num_filter = EEPROM.readUInt(i*sizeof(uint32_t));if(num_filter>3||num_filter<0)num_filter=0;i++;
}

void writeConfig(int numpar,uint32_t value){
  value_button[numpar]=value;
  EEPROM.writeUInt(numpar*sizeof(uint32_t),value);
  EEPROM.commit();
}
