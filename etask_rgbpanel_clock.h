#ifndef ETASK_RGBPANEL_CLOCK
#define ETASK_RGBPANEL_CLOCK

/* Platformio Libraries needed:
lib_deps =
    adafruit/Adafruit GFX Library @ ^1.11.3
    mrfaptastic/ESP32 HUB75 LED MATRIX PANEL DMA Display @ ^2.0.7
*/
//Pinout
#define R1_PIN  (25)
#define G1_PIN  (26)
#define B1_PIN  (27)
#define R2_PIN  (14)
#define G2_PIN  (12)
#define B2_PIN  (13)
#define CLK_PIN (19)
#define LAT_PIN (32)
#define OE_PIN  (33)
#define A_PIN   (15)
#define B_PIN   (18)
#define C_PIN   (17)
#define D_PIN   (16)
#define E_PIN   (-1)

#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>

#define PANEL_RES_X 64      // Number of pixels wide of each INDIVIDUAL panel module. 
#define PANEL_RES_Y 32     // Number of pixels tall of each INDIVIDUAL panel module.
#define PANEL_CHAIN 1      // Total number of panels chained one to another

void etask_rgbpanel_clock(void * parameters);

#endif