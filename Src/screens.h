


void draw_Grid(int px, int py, int w, int h )
{
  int div = (int)(h / 5);
  for (int x = 0; x < w + div; x += div)tft.line(px + x, py, px + x, py + h, 0b010101);
  for (int y = 0; y < h + div; y += div)tft.line(px, py + y, px + w, py + y, 0b010101);
}

void draw_spectr(){//спектр
      int x = 0;
      int y = 0;
      uint8_t col_s = 10;
      static int stolb[NUM_SAMPLE_BUF/2] = {0,};
      int width_b = bandwidth/(I2S_SAMPLE_RATE/NUM_SAMPLE_BUF);
      int indent_b = indent/(I2S_SAMPLE_RATE/NUM_SAMPLE_BUF);
     for (int i = 14; i<NUM_SAMPLE_BUF/2;i++){
       switch(rf_mode){
        case LSB:if((i<=pos_fft-indent_b)&&(i>=pos_fft-width_b-indent_b)){col_s=colors[9];}else{col_s=colors[10];}break;
        case USB:if((i>=pos_fft+indent_b)&&(i<=pos_fft+width_b+indent_b)){col_s=colors[9];}else{col_s=colors[10];}break;
        case  AM:if(((i<=pos_fft-indent_b)&&(i>=pos_fft-width_b-indent_b))||((i>=pos_fft+indent_b)&&(i<=pos_fft+width_b+indent_b))){col_s=colors[9];}else{col_s=colors[10];}break;
       }
      if(y>175)y=175;
      tft.fillRect(x*m_screen,175-stolb[i],2,stolb[i],BLACK);
      y = 4*sqrtf(fft[i]);stolb[i]=y;
      tft.dotFast(x*m_screen,175-y,colors[16]);
      tft.dotFast(x*m_screen+1,175-y,colors[16]);
      tft.fillRect(x*m_screen,177-y,2,y,col_s);
      x++;if(x*m_screen>480){break;}
     }
}

void draw_waterfall(){ //отображаем массив буферов fft для водопада
  uint8_t y = 200;
  int x = 0;
  for (int i=WP_LINE;i>0;i--){
    x=0;
    for(int f=14;f<NUM_SAMPLE_BUF/2;f++){
      tft.xLine(x*m_screen,x*m_screen+m_screen,i+y,wp[wp_num[i-1]][f]);
      if (abs(pos_fft - f)<2){tft.fillRect(x*m_screen,y+1,1,WP_LINE,colors[5]);}
      if (x*m_screen>480)break;
      x++;
    }
  }
}


void draw_service(){
  static int sss = 0;
  uint8_t col_s;
  char str[32];
  const char* mod;const char* filtr;const char* level;const char* tun;const char* type_fir;const char* gain;
  int c_freq = freq+((float)I2S_SAMPLE_RATE/(float)NUM_SAMPLE_BUF)*pos_fft; //
  sprintf(str, "%2d %03d,%02d",  c_freq/1000000, (c_freq/1000)%1000, (c_freq/10)%100 );
  tft.rect(1,1,230,70,colors[2]);tft.rect(230,1,249,70,colors[2]);
  if (rf_mode == LSB) mod = "LSB";
  if (rf_mode == USB) mod = "USB";
  if (rf_mode == AM)  mod = "AM ";
  if (num_filter == 0) filtr = "3000";
  if (num_filter == 1) filtr = "2400";
  if (num_filter == 2) filtr = " 500";
  if (num_filter == 3) filtr = "6000";
  if (tuning){tun = "ROT";}else{tun = "PAN";}
  if (filter_gain==0) gain = "  0db";
  if (filter_gain==1) gain = " +3db";
  if (filter_gain==2) gain = "+10db";
  tft.setFont(CodePage437_8x14);
  tft.setTextColor(colors[5],BLACK);
  tft.setCursor(10,7);tft.print("BAND");
  tft.setCursor(10,22);tft.print("IFg+");
  tft.setCursor(10,37);tft.print("IFg-");
  tft.setCursor(10,52);tft.print("STEP");
  tft.setCursor(438,7);tft.print(filtr);
  tft.setCursor(445,22);tft.print(mod);
  tft.setCursor(445,52);tft.print(tun);
  tft.setCursor(431,37);tft.print(gain);

  gfx.setFont(&Seven_Segment18pt8b);gfx.setTextSize(1);
  gfx.setTextColor(RGB565(colors[8]),RGB565(colors[0]));
  if (redraw_freq){tft.fillRect(260,5,150,40,RGB565(colors[0]));redraw_freq=false;}
  gfx.setCursor(260,40);gfx.print(str);
  tft.setFont(CodePage437_8x14);
  tft.setCursor(70,55);tft.setTextColor(colors[15],BLACK);
  tft.print("IFgain:");tft.print(If_gain);tft.print(" ");
  tft.setCursor(165,55);
  tft.print("AGC:");tft.print((int)agc_koeff);tft.print(" ");
  tft.setCursor(260,55);tft.setTextColor(colors[8],BLACK);
  tft.print("Step: ");tft.print(step_freq);tft.print(" Hz      ");
  //tft.setTextColor(0b11001100,0);
  //tft.setCursor(380,55);tft.print(bands[numband].name);
  
  if(old_smeter > 35) col_s = colors[5];
  if(old_smeter > 50) col_s = colors[1];
  if(old_smeter <= 35) col_s = colors[4];
  level = "9+60";
  if((old_smeter>0)&&(old_smeter<10))level = "1   ";
  else if((old_smeter>9)&&(old_smeter<15))level = "2   ";
  else if((old_smeter>14)&&(old_smeter<20))level = "3   ";
  else if((old_smeter>19)&&(old_smeter<25))level = "4   ";
  else if((old_smeter>24)&&(old_smeter<30))level = "5   ";
  else if((old_smeter>29)&&(old_smeter<35))level = "6   ";
  else if((old_smeter>34)&&(old_smeter<40))level = "7   ";
  else if((old_smeter>39)&&(old_smeter<45))level = "8   ";
  else if((old_smeter>44)&&(old_smeter<50))level = "9   ";
  else if((old_smeter>49)&&(old_smeter<55))level = "9+20";
  else if((old_smeter>54)&&(old_smeter<60))level = "9+30";
  else if((old_smeter>59)&&(old_smeter<65))level = "9+40";
  else if((old_smeter>64)&&(old_smeter<70))level = "9+60";
  else if(old_smeter>69){level = "9+70";old_smeter = 75;}
  tft.setCursor(130,7);tft.setTextColor(colors[19],BLACK);
  tft.print("S:");tft.print(level);tft.print(" ");
  tft.fillRect(70,27,old_smeter*2,10,col_s);
  tft.fillRect(70+old_smeter*2,27,150-old_smeter*2,10,colors[6]);
  gfx.drawRGBBitmap(70,40,sMeter_c,155,11);
}

void draw_band(){
  uint16_t s_col = RGB565(0b11001010);
  if(show_band_time>1){
    gfx.setTextColor(s_col,RGB565(BLACK));
    gfx.setFont(&Seven_Segment18pt8b);
    gfx.setTextSize(2);
    gfx.setCursor(180,130);
    gfx.print(bands[numband].name);
    redraw=false;
  }
   
}
#ifdef DEBUG_RUN
void draw_debug(){
    tft.setTextColor(WHITE,BLACK);
    tft.setCursor(330,75);tft.print("Tasks(wait/run) uS");
    tft.setCursor(330,97);tft.print("Rx_in: ");
    tft.print(rx_in_wait_result);tft.print("/");tft.print(rx_in_run_result);tft.print("   ");
    tft.setCursor(330,119);tft.print("Rx_dsp:");
    tft.print(rx_dsp_wait_result);tft.print("/");tft.print(rx_dsp_run_result);tft.print("   ");
    tft.setCursor(330,141);tft.print("Rx_out:");
    tft.print(rx_out_wait_result);tft.print("/");tft.print(rx_out_run_result);tft.print("   ");
    
}
#endif
void rx_display(){
   if(redraw){tft.clear(0);redraw=false;}
   draw_spectr();    //отобразить спектр
   draw_band();
   draw_Grid(0,72,480,105);
   #ifdef DEBUG_RUN
    draw_debug();
   #endif
   draw_waterfall(); //отобразить "водопад"
   draw_service();   //отобразить остальное
   tft.show();
}

void setup_key_display(){
  if(redraw){tft.clear();redraw=false;}
  uint16_t lmm = 0;
  uint16_t rmm = 0;
  for(int i=0;i<10;i++){
    lmm += adc1_get_raw(LEFT_ADC);rmm += adc1_get_raw(RIGHT_ADC);
  }
  if(n_button<4)value_adc=lmm/10;if(n_button>3 && n_button<8)value_adc=rmm/10;
  tft.setFont(CodePage437_8x14);
  tft.setCursor(100,180);tft.setTextColor(YELLOW);
  tft.print("Set the cursor on the desired button\n        and press and hold.\n    Click the encoder to save.");
  tft.setTextColor(WHITE);
  switch(n_button){
    case 0:tft.fillRect(55,21,10,32,RED);tft.fillRect(10,22,38,30,BLACK);break;
    case 1:tft.fillRect(55,84,10,32,RED);tft.fillRect(10,85,38,30,BLACK);break;
    case 2:tft.fillRect(55,152,10,32,RED);tft.fillRect(10,153,38,30,BLACK);break;
    case 3:tft.fillRect(55,216,10,32,RED);tft.fillRect(10,217,38,30,BLACK);break;
    case 4:tft.fillRect(420,21,10,32,RED);tft.fillRect(436,22,38,30,BLACK);break;
    case 5:tft.fillRect(420,84,10,32,RED);tft.fillRect(436,85,38,30,BLACK);break;
    case 6:tft.fillRect(420,152,10,32,RED);tft.fillRect(436,153,38,30,BLACK);break;
    case 7:tft.fillRect(420,216,10,32,RED);tft.fillRect(436,217,38,30,BLACK);break;
    case 8:tft.fillRect(300,115,10,42,RED);break;
    }
  tft.rect(9,21,40,32,(GREEN));tft.setCursor(18,29);tft.print(n_button == 0 ? value_adc:value_button[0]);
  tft.rect(9,84,40,32,(YELLOW));tft.setCursor(18,93);tft.print(n_button == 1 ? value_adc:value_button[1]);
  tft.rect(9,152,40,32,(CYAN));tft.setCursor(18,160);tft.print(n_button == 2 ? value_adc:value_button[2]);
  tft.rect(9,216,40,32,(MAGENTA));tft.setCursor(18,226);tft.print(n_button == 3 ? value_adc:value_button[3]);
  tft.rect(435,21,40,32,(GREEN));tft.setCursor(444,29);tft.print(n_button == 4 ? value_adc:value_button[4]);
  tft.rect(435,84,40,32,(YELLOW));tft.setCursor(444,93);tft.print(n_button == 5 ? value_adc:value_button[5]);
  tft.rect(435,152,40,32,(CYAN));tft.setCursor(444,160);tft.print(n_button == 6 ? value_adc:value_button[6]);
  tft.rect(435,216,40,32,(MAGENTA));tft.setCursor(444,226);tft.print(n_button == 7 ? value_adc:value_button[7]);
  gfx.drawRoundRect(190,115,100,42,5,RGB565(WHITE));
  gfx.setTextColor(RGB565(WHITE),RGB565(BLACK));gfx.setFont(&Seven_Segment18pt8b);
  gfx.setCursor(212,146);gfx.print("EXIT");
  if(n_button==8){flag_exit_setup=true;}else{flag_exit_setup=false;}
  if(flag_write_config && value_adc<500)writeConfig(n_button,value_adc);
  flag_write_config=false;
  
}

void screens(uint8_t s){
  switch (s){
    case 0:break;
    case 1:rx_display();break;
    case 2:setup_key_display();break;
    case 3:break;
 }
}
