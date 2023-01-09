
uint8_t lastkey_l(uint16_t value){
    uint8_t key = 0;
    if ((value > 500)){key=0;return key;}
    else if (abs(value - value_button[0]) < 5){key=1;return key;}
    else if (abs(value - value_button[1]) < 5){key=2;return key;}
    else if (abs(value - value_button[2]) < 5) {key=3;return key;}
    else if (abs(value - value_button[3]) < 5) {key=4;return key;}
    return key;
}

uint8_t lastkey_r(uint16_t value){
    uint8_t key = 0;
    if ((value > 500)){key=0;return key;}
    else if (abs(value - value_button[4]) < 5){key=1;return key;}
    else if (abs(value - value_button[5]) < 5){key=2;return key;}
    else if (abs(value - value_button[6]) < 5) {key=3;return key;}
    else if (abs(value - value_button[7]) < 5) {key=4;return key;}
    return key;
}

//возвращает усредн.данные АЦП (0 - channel0, 1 - channel3)
uint16_t v_knopka(int c){
       uint16_t ll = 0;uint16_t mm = 0;
       for (int i = 0;i < 5;i++){ //5 опросов для усреднения
        if (c == 0){mm = adc1_get_raw(LEFT_ADC);}else{mm = adc1_get_raw(RIGHT_ADC);}
        ll = ll + mm;
       }
       return (ll/5);
}

//опрос левых кнопок
void l_knopka(){
       uint16_t value = v_knopka(0);
       if ((value > 500) && l_press) {l_release = true;return;}//л.кнопка отпущена
       in_left = v_knopka(0);//вторичный запрос данных АЦП для последующего сравнения и исключения ложных
       if ((in_left  < 500) && (abs(in_left - value) < 5))
       {l_release = false;l_press = true;lkey = lastkey_l(in_left);}else{l_press = false;}
}

//опрос правых кнопок
void r_knopka(){
       uint16_t value = v_knopka(1);
       if ((value > 500) && r_press) {r_release = true;return;}//пр.кнопка отпущена
       in_right = v_knopka(1);//вторичный запрос данных АЦП для последующего сравнения  и исключения ложных
       if ((in_right < 500)  && (abs(in_right - value) < 5))
       {r_release = false;r_press = true;rkey = lastkey_r(in_right);}else{r_press = false;}
}
