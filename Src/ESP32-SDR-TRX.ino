#include <driver/adc.h>
#include <driver/i2s.h>
#include <EEPROM.h>
#include <Wire.h>
#include <si5351.h> //https://github.com/etherkit/Si5351Arduino
#include <PCF8574.h>//https://github.com/xreef/PCF8574_library
#undef DEBUG_PRINTLN
#undef DEBUG_PRINT
#include <AiEsp32RotaryEncoder.h> //https://github.com/igorantolic/ai-esp32-rotary-encoder
#include <ESP32Lib.h>   //https://github.com/bitluni/ESP32Lib
#include <GfxWrapper.h> //должна быть установлена библиотека GFX-Adafruit https://github.com/adafruit/Adafruit-GFX-Library
#include <Ressources/CodePage437_8x14.h>
#include "src/dsp_lib/xtensa_math.h" //taken from https://github.com/whyengineer/esp32-lin/tree/master/components/dsp_lib
#include "include/s7.h"          //7-segments font
#include "include/images.h"
#include "wm8731.h"
#include "global.h"
#include "fft.h"
#include "filters.h"
#include "init.h"
#include "key.h"
#include "rx.h"
#include "tx.h"
#include "screens.h"
#include "tools.h"




void setup() {

 Serial.begin(115200,SERIAL_8N1,-1,TX_SERIAL);//только serial-TX
 pinMode(PTT,INPUT_PULLUP);
 EEPROM.begin(sizeof(uint32_t)*32);
 readConfig(); //восстанавливаем значения
 
 vSemaphoreCreateBinary(xRXDSP);
 vSemaphoreCreateBinary(xRXIN);
 vSemaphoreCreateBinary(xRXOUT);
 vSemaphoreCreateBinary(xTXDSP);
 vSemaphoreCreateBinary(xTXIN);
 vSemaphoreCreateBinary(xTXOUT);
 
 rotaryEncoder.begin();
 pinMode(ROTARY_ENCODER_BUTTON_PIN,INPUT_PULLUP);
  rotaryEncoder.setup(
    [] { rotaryEncoder.readEncoder_ISR(); },
    [] { NULL; });
 rotaryEncoder.disableAcceleration();
 Wire.begin(I2C_SDA,I2C_SCL);
 Wire.setClock(400000);
 si = si5351.init(SI5351_CRYSTAL_LOAD_8PF, 0, 0);
 if(si){si5351.set_freq(1228800000L,SI5351_CLK0);//to do clock wm8731 in master-mode
       si5351.output_enable(SI5351_CLK0, 1);
       Serial.println("SI5351 OK");}
 else{Serial.println("SI5351 not OK");}
 wm8731_init(); 
 pcf_init(); 
 adc_init();
 fft_init();
 buf_init();
 i2sinit();//I2S0
 xTaskCreatePinnedToCore(rx_in,  "rxin",  2048, NULL, 100, NULL, 1);//
 xTaskCreatePinnedToCore(rx_out, "rxout", 2048, NULL, 110, NULL, 1);//
 xTaskCreatePinnedToCore(rx_dsp, "rxdsp", 2048, NULL, 220, NULL, 0);//
 xTaskCreatePinnedToCore(tx_in,  "txin",  2048, NULL, 221, NULL, 1);//
 xTaskCreatePinnedToCore(tx_out, "txout", 2048, NULL, 109, NULL, 1);//
 xTaskCreatePinnedToCore(tx_dsp, "txdsp", 2048, NULL, 219, NULL, 0);//

 #ifdef VGA
  tft.init(myMode,RED0,RED1,GREEN0,GREEN1,BLUE0,BLUE1,HSYNCPIN,DEPIN);
 #else
  tft.init(tft.MODE480x272,RED0,RED1,GREEN0,GREEN1,BLUE0,BLUE1,HSYNCPIN,DEPIN,CLOCKPIN);
 #endif
 start_ok();
 //нажатие энкодера во время заставки вызывает экран настройки кнопок
 //нажатие энкодера во время приема сохраняет основные настройки для их восстановления при последующем включении
}

void loop() {
  control();
  screens(current_mode);
}
