#ifndef WM8731_H
#define WM8731_H

#include <stdint.h>
#include "Arduino.h"
#include <Wire.h>

#define I2C_SCL 12
#define I2C_SDA 14

#define WM8731_WRITE_ADDRESS               0x1A
#define WM8731_READ_ADDRESS                0x1B

// WM8731 register map
#define WM8731_LEFT_LINE_IN                0x00
#define WM8731_LRINBOTH                    0x100
#define WM8731_LINMUTE                     0x080
#define WM8731_LINVOL                      0x01F

#define WM8731_RIGHT_LINE_IN               0x01
#define WM8731_RLINBOTH                    0x100
#define WM8731_RINMUTE                     0x080
#define WM8731_RINVOL                      0x01F

#define WM8731_LEFT_HP_OUT                 0x02
#define WM8731_LRHPBOTH                    0x100
#define WM8731_LZCEN                       0x080
#define WM8731_LHPVOL                      0x07F

#define WM8731_RIGHT_HP_OUT                0x03
#define WM8731_RLHPBOTH                    0x100
#define WM8731_RZCEN                       0x080
#define WM8731_RHPVOL                      0x07F

#define WM8731_ANALOG_AUDIO_PATH_CONTROL   0x04
#define WM8731_SIDEATT_1                   0x080
#define WM8731_SIDEATT_0                   0x040
#define WM8731_SIDETONE                    0x020
#define WM8731_DACSEL                      0x010
#define WM8731_BYPASS                      0x008
#define WM8731_INSEL                       0x004
#define WM8731_MUTEMIC                     0x002
#define WM8731_MICBOOST                    0x001

#define WM8731_SIDEATT_Pos                 6

#define WM8731_DIGITAL_AUDIO_PATH_CONTROL  0x005
#define WM8731_HPOR                        0x010
#define WM8731_DACMU                       0x008
#define WM8731_DEEMPH_1                    0x004
#define WM8731_DEEMPH_0                    0x002
#define WM8731_ADCHPD                      0x001

#define WM8731_DEEMPH_Pos                  1

#define WM8731_POWER_DOWN_CONTROL          0x06
#define WM8731_POWEROFF                    0x080
#define WM8731_CLKOUTPD                    0x040
#define WM8731_OSCPD                       0x020
#define WM8731_OUTPD                       0x010
#define WM8731_DACPD                       0x008
#define WM8731_ADCPD                       0x004
#define WM8731_MICPD                       0x002
#define WM8731_LINEINPD                    0x001

#define WM8731_DIGITAL_AUDIO_INTERFACE_FORMAT  0x07
#define WM8731_BCLKINV                     0x080
#define WM8731_MS                          0x040
#define WM8731_LRSWAP                      0x020
#define WM8731_LRP                         0x010
#define WM8731_IWL_1                       0x008
#define WM8731_IWL_0                       0x004
#define WM8731_FORMAT_1                    0x002
#define WM8731_FORMAT_0                    0x001

#define WM8731_IWL_Pos                     2
#define WM8731_FORMAT_Pos                  0

#define WM8731_SAMPLING_CONTROL            0x08
#define WM8731_CLKODIV2                    0x080
#define WM8731_CLKIDIV2                    0x040
#define WM8731_SR_3                        0x020
#define WM8731_SR_2                        0x010
#define WM8731_SR_1                        0x008
#define WM8731_SR_0                        0x004
#define WM8731_BOSR                        0x002
#define WM8731_USB_NORMAL                  0x001

#define WM8731_ACTIVE_CONTROL              0x09
#define WM8731_ACTIVE                      0x001

#define WM8731_RESET                       0x0F

typedef enum {
    LEFT,
    RIGHT,
    BOTH 
} WM8731_Channel;

void wm8731_init();
void wm8731_write_reg(uint8_t reg, uint8_t value);
void wm8731_set_line_in_volume(uint8_t vol);

#endif
