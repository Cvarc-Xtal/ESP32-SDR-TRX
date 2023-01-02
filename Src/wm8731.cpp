#include "wm8731.h"


void wm8731_init() {

    wm8731_write_reg(0b00011110, 0b00000000); //R15 Reset Chip
    wm8731_write_reg(0b00000100, 0b01111111); //R2 Left Headphone Out Mute
    wm8731_write_reg(0b00000110, 0b01111111); //R3 Right Headphone Out Mute
    wm8731_write_reg(0b00001110, 0b10111110); //R7 Digital Audio Interface Format,
                                                 //Codec Slave, 32bits, I2S Format,
                                                 //MSB-First left-1 justified
    wm8731_write_reg(0b00010000, 0b00000010); //R8 Sampling Control normal mode, 384fs,
 
    wm8731_write_reg(0b00000000, 0b00011111); //R0 Left Line Input volume
    wm8731_write_reg(0b00000010, 0b00011111); //R1 Right Line Input volume
    wm8731_write_reg(0b00001000, 0b00010010); //R4 Analogue Audio Path Control
    wm8731_write_reg(0b00001010, 0b00000110); //R5 Digital Audio Path Control
    wm8731_write_reg(0b00001100, 0b01100000); //R6 Power Down Control
    wm8731_write_reg(0b00010010, 0b00000001); //R9 reactivate digital audio interface

}

void wm8731_write_reg(uint8_t reg, uint8_t value) {
    uint8_t st = 2;
    uint8_t repeats = 0;
    Wire.beginTransmission(WM8731_WRITE_ADDRESS);
    Wire.write(reg);Wire.write(value);
    st = Wire.endTransmission();
}

void wm8731_reset() {
    wm8731_write_reg(0b00011110, 0b00000000); //R15 Reset Chip
}


// set the line input volume to vol
// vol: from 0 (=-34.5 dB) to 31 (=+12 dB) in 1.5 dB steps
void wm8731_set_line_in_volume(uint8_t vol) {
    static uint8_t old_vol = 0;
    if(vol == old_vol)return;
    wm8731_write_reg(0b00000000, vol); //R0 Left Line In
    wm8731_write_reg(0b00000010, vol); //R1 Right Line In
    old_vol=vol;
}
