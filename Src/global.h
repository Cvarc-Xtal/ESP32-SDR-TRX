#define VERSION "v1.0"


#define LCD_DE  //Для дисплея с синхронизацией DE-only.Если дисплей синхронизируется через Hsync/Vsync, то закомментировать

#ifdef LCD_DE
  #include "lcd/LCD6BitI.h"    //драйвер дисплея 4,3",480x272,LB043WQ2-TD01 (sync DE) https://github.com/Cvarc-Xtal/esp32tft24
#else
  #include "lcd/LCD6BitIS.h"   //драйвер дисплея 4,3",480x272,LMS430HF02 (sync HSync/VSync) https://github.com/Cvarc-Xtal/esp32tft24
#endif

//Возможен вариант с VGA дисплеем с максимальным разрешение до 500x300 (ограничение по размеру памяти)

typedef struct FIR {
    float  *coeffs;
    float  *delay;
    int     N;
    int     pos;
};

float acc_hpf[5];
float acc_lpf[5];

struct COMPLEX
{
 float re;
 float im;
};

struct COMPLEX_int
{
  int re;
  int im;
};

#define I2S_SAMPLE_RATE (48000)    //частота дискретизации
#define NUM_SAMPLE_BUF  (512)      //размер входного буфера (512 only!)
#define NUM_FFT_BUF     (NUM_SAMPLE_BUF*2)

struct  COMPLEX_int input_buffer[NUM_SAMPLE_BUF]; //входной I2S-буфер
struct  COMPLEX_int output_buffer[NUM_SAMPLE_BUF];//выходной I2S-буфер
struct  COMPLEX workbuf_in[NUM_FFT_BUF];          //рабочий буфер
struct  COMPLEX workbuf_out[NUM_FFT_BUF/2];       //промежуточный выходной буфер
struct  COMPLEX workbuf_tmp[NUM_FFT_BUF/2];       //временный буфер
float workbuf_re[NUM_FFT_BUF/2];//рабочий буфер tx re
float workbuf_im[NUM_FFT_BUF/2];//рабочий буфер tx im

extern "C" {
  int fir_f32(FIR* fir, float* input, float* output, int len);
  int iir_biquad_f32(float* input, float* output, int len, float* coef, float* acc);
}
void IRAM_ATTR rx_in (void * pvParameters);
void IRAM_ATTR rx_out(void * pvParameters);
void IRAM_ATTR rx_dsp(void * pvParameters);
void IRAM_ATTR tx_in (void * pvParameters);
void IRAM_ATTR tx_out(void * pvParameters);
void IRAM_ATTR tx_dsp(void * pvParameters);
void control();
void fft_init();
void i2sinit();
void adc_init();
void pcf_init();
void control();
void readConfig();
void writeConfig(int numpar, uint32_t value);
void buf_init();
void start_ok();

#ifdef  LCD_DE
  LCD6BitI tft; //
  GfxWrapper<LCD6BitI> gfx(tft,480,272); //включение совместимости с GFX-Adafruit можно использовать все функции GFX-Adafruit через класс gfx.
                                //hfront,hsync,hback,pixels,vfront,vsync,vback,lines,divy,pixelclock,hpolaritynegative,vpolaritynegative
  const Mode LCD6BitI::MODE480x272(0, 48,  0, 480,  0, 14, 0, 272, 1, 0, 1, 1); //для LB043WQ2-TD01 sync DE only
#else
  LCD6BitIS tft;
  GfxWrapper<LCD6BitIS> gfx(tft,480,272); //включение совместимости с GFX-Adafruit можно использовать все функции GFX-Adafruit через класс gfx.
                                 //hfront,hsync,hback,pixels,vfront,vsync,vback,lines,divy,pixelclock,hpolaritynegative,vpolaritynegative
  const Mode LCD6BitIS::MODE480x272(0, 54, 42, 480, 1, 2, 12, 272, 1, 0, 0, 0); //для LMS430HF02 sync Hsync/Vsync
#endif

//подключение дисплея-(тач не используется)
#ifdef  LCD_DE
//пины ESP32         |пины lcd LB043WQ2-TD01
//-------------------|-------------------------------
#define R0       21    // 5+7+9+11 мл.разряд красного
#define R1       19    // 6+8+10+12 ст.разряд красного
#define G0       18    // 13+15+17+19 мл.разряд зеленого
#define G1        5    // 20+22+24+26 ст.разряд зеленого
#define B0       17    // 21+23+25+27 мл.разряд синего
#define B1       16    // 22+24+26+28 ст.разряд синего
#define HSYNCPIN -1    // не выделять пин для hsync
#define DEPIN    15    // 34 Синхронизация без hSync/vSync. Только DE
#define CLOCKPIN  4    // 30 clock

#else

//пины ESP32         |пины lcd LMS430HF02
//-------------------|-------------------------------
#define R0       21    // 5+7+9+11 мл.разряд красного
#define R1       19    // 6+8+10+12 ст.разряд красного
#define G0       18    // 13+15+17+19 мл.разряд зеленого
#define G1        5    // 20+22+24+26 ст.разряд зеленого
#define B0       17    // 21+23+25+27 мл.разряд синего
#define B1       16    // 22+24+26+28 ст.разряд синего
#define HSYNCPIN  2    // 32 hsync
#define DEPIN    15    // 33 vsync
#define CLOCKPIN  4    // 30 clock
                       // 34(DE pin) -> GND (некоторые модели LCD требуют установить в 1)
#endif

#define TX_SERIAL 1

#define PTT 13 //LOW-передача HIGH-прием

//ADC для кнопок
#define LEFT_ADC  (ADC1_CHANNEL_0) //GPIO36
#define RIGHT_ADC (ADC1_CHANNEL_3) //GPIO39

//GPIO для I2S цап/ацп
//GPIO0 присутствует не на всех модулях ESP32. Для MCLK можно использовать также GPIO1 или GPIO3
#define I2S_MCLK  3   //0, 1 or 3 only
#define I2S_BCK   25
#define I2S_WS    26
#define I2S_DOUT  23
#define I2S_DIN   22

//GPIO I2C pins
#define I2C_SCL 12
#define I2C_SDA 14

//Encoder pins
#define ROTARY_ENCODER_A_PIN      34
#define ROTARY_ENCODER_B_PIN      35
#define ROTARY_ENCODER_BUTTON_PIN 27
#define ROTARY_ENCODER_VCC_PIN    -1
#define ROTARY_ENCODER_STEPS       4

#define WP_LINE 70  //кол-во строк "водопада"
#define CWW  20     //кол-во цветов для "водопада"
#define FWW  140    //значение амплитуды  для "максимального" цвета

uint8_t wp[WP_LINE][NUM_SAMPLE_BUF/2];//массив строк для водопада[кол-во строк][кол-во точек в строке]
uint8_t wp_num[WP_LINE];          //массив для хранения порядка номеров строк водопада для вывода на экран
float  fft[NUM_SAMPLE_BUF];       //рабочий fft-буфер
float  fft_inter[NUM_SAMPLE_BUF]; //отображаемый fft буфер (все элементы постоянно уменьшаются)

#define PCH (NUM_SAMPLE_BUF/4) //середина отображаемомго спектра ("ПЧ")
int pos_fft = PCH; //позиция указателя на панораме/спектре
int m_screen = 2; // множитель масштабирования панорамы/спектра

float avg_fft = 0.0f;
bool dec_Ifgain = false;
bool inc_Ifgain = false;
float agc_koeff = 1.5f;
bool agc = true; //вкл/выкл ару (регулировка по выходу кодека)
uint8_t If_gain = 31; //усиление по входу кодека. 31-максимум
uint8_t filter_gain = 0;

int bandwidth; //текущая полоса пропускания, отображаемая на спектре
int indent = 0;

uint8_t colors_w[CWW] = {//набор цветов для "водопада"
  0b010000,
  0b100000,
  0b100001,
  0b100100,
  0b100101,
  0b110100,
  0b110101,
  0b101000,
  0b101001,
  0b101100,
  0b101101,
  0b111100,
  0b111101,
  0b001000,
  0b011100,
  0b001010,
  0b001110,
  0b001111,
  0b010110,
  0b010111
};

uint8_t colors[] = { //
  0b11000010, //0 
  0b11000011, //1
  0b11001010, //2
  0b11001011, //3
  0b11001110, //4
  0b11001111, //5
  0b11001000, //6
  0b11001100, //7
  0b11101100, //8
  0b11101000, //9
  0b11100000, //10
  0b11110000, //11
  0b11100010, //12
  0b11110010, //13
  0b11110011, //14
  0b11111010, //15
  0b11101110, //16
  0b11101011, //17
  0b11101010, //18
  0b11111111  //19
  };

#define GREEN   0b11001100
#define BLUE    0b11110000
#define RED     0b11000011
#define MAGENTA 0b11110011
#define CYAN    0b11111100
#define YELLOW  0b11001111
#define WHITE   0b11111111
#define BLACK   0b11000000

//макрос-конвертер цвета для GFXWrapper(R2G2B2 -> R5G6B5)
#define RGB565(ccc)        (uint16_t)((ccc << 0xE)|((ccc & 0xC)<<7)|((ccc & 0x30)>>1))

unsigned long cur_ms   = 0;

uint16_t in_left = 999; //знач.ацп лев.кнопок
uint16_t in_right = 999;//знач.ацп пр.кнопок
bool l_release = false; //признак отпущенной лев.кнопки
bool r_release = false; //признак отпущенной пр.кнопки
bool l_press = false;   //признак нажатой лев.кнопки
bool r_press = false;   //признак нажатой пр.кнопки
uint8_t lkey = 0;       //код лев.кнопки для дальн.действия 1..5 (0-нет действия)
uint8_t rkey = 0;       //код пр..кнопки для дальн.действия 1..5 (0-нет действия)
volatile bool key_rotary = false;
int16_t encoderDelta = 0;

bool redraw = false;
bool fill_fft = true;
bool redraw_freq = false;

bool tuning = false; //
uint8_t rf_mode = 0;//0-LSB, 1-USB, 2-AM
uint8_t num_filter = 0; //номер текущего фильтра основной селекции
int32_t smeter = 0;
int32_t old_smeter = 50;
uint32_t freq = 14200000; //частота настройки трансивера
int step_freq = 100; //шаг перестройки гц
int numband = 8;//номер текущего диапазона
bool speak_out = false;
bool si = false;

int n_button = 0;
uint16_t value_button[9]={0,0};
bool flag_exit_setup=false;
uint16_t value_adc = 0;
bool flag_write_config = false;
bool flag_write_parameters = false;

#define SHOW_BAND 15  //время показа диапазона сек
int show_band_time = SHOW_BAND;


struct band {
  unsigned long long freq; // frequency in Hz
  char* name;        // name of band
  int mode;
  int filter;
};

const int LSB = 0;
const int USB = 1;
const int AM  = 2;
const int TX_MODE = 0;
const int RX_MODE = 1;
const int SETUP_MODE = 2;

uint8_t current_mode = RX_MODE;      //1-прием, 0-передача, 2-настройка

#define N_BANDS 13
struct band bands[N_BANDS] = {
  1850000,"160M" ,LSB,0,
  3700000,"80M " ,LSB,0,
  3995000,"75M " ,AM, 3,
  4850000,"60M " ,LSB,0,
  5840000,"49M " ,AM, 3,
  7100000,"40M " ,LSB,0,
  9520000,"31M " ,AM, 3,
  11670000,"25M ",AM, 3,
  14200000,"20M ",USB,0,
  17780000,"16M ",AM, 3,
  21200000,"15M ",USB,0,
  24920000,"12M ",AM, 3,
  28350000,"10M ",USB,0
};
