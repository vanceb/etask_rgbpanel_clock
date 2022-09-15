#include <Arduino.h>
#include <WiFi.h>
#include <uptime.h>

#define COLOR_VFD       225
#define COLOR_RED       84
#define DISPLAY_COLOR   84     // Define a singe colour from the color wheel for the display
//#define SHOW_UPTIME 1
//#define SHOW_COLOR  1
#define SHOW_DAY 1
#define SECONDS_BAR 1

#include <etask_rgbpanel_clock.h>
#include <Fonts/FreeMonoBold12pt7b.h>

#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>

MatrixPanel_I2S_DMA *dma_display = nullptr;

PROGMEM static char days[7][10] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
// From: https://gist.github.com/davidegironi/3144efdc6d67e5df55438cc3cba613c8
uint16_t colorWheel(uint8_t pos) {
  if(pos < 85) {
    return dma_display->color565(pos * 3, 255 - pos * 3, 0);
  } else if(pos < 170) {
    pos -= 85;
    return dma_display->color565(255 - pos * 3, 0, pos * 3);
  } else {
    pos -= 170;
    return dma_display->color565(0, pos * 3, 255 - pos * 3);
  }
}

void etask_rgbpanel_clock(void *parameters)
{
    HUB75_I2S_CFG::i2s_pins _pins = {R1_PIN,
                                     G1_PIN,
                                     B1_PIN,
                                     R2_PIN,
                                     G2_PIN,
                                     B2_PIN,
                                     A_PIN,
                                     B_PIN,
                                     C_PIN,
                                     D_PIN,
                                     E_PIN,
                                     LAT_PIN,
                                     OE_PIN,
                                     CLK_PIN};

    // Module configuration
    HUB75_I2S_CFG mxconfig(
        PANEL_RES_X, // module width
        PANEL_RES_Y, // module height
        PANEL_CHAIN, // Chain length
        _pins        // Custom pins
    );
    mxconfig.double_buff = true; // Turn on double buffer
    mxconfig.clkphase = false;
    mxconfig.driver = HUB75_I2S_CFG::FM6126A;

    // Display Setup
    dma_display = new MatrixPanel_I2S_DMA(mxconfig);
    dma_display->begin();
    dma_display->setBrightness8(10); // 0-255
    dma_display->clearScreen();
    dma_display->flipDMABuffer();

    /* Run forever - Required for FreeRTOS task */
    struct tm timeinfo;
    unsigned long start = 0;
    unsigned long loopcounter = 0;
    long dly = 0;
    uint16_t color = 0xF800;

    for (;;)
    {
        start = millis();
        getLocalTime( &timeinfo );

        dma_display->flipDMABuffer();
        dma_display->clearScreen();
        // Valid only if year > 2000. 
        // You can get from timeinfo : tm_year, tm_mon, tm_mday, tm_hour, tm_min, tm_sec
        if (timeinfo.tm_year > 100 )
        {
            /*
            //dma_display->setTextSize(2);     // size 1 == 8 pixels high
            dma_display->setTextColor(0xF100);
            dma_display->setFont(&FreeMonoBold12pt7b);     // size 1 == 8 pixels high
            dma_display->setTextWrap(false); // Don't wrap at end of line - will do ourselves
            dma_display->setCursor(0, 16);    // start at top left, with 8 pixel of spacing
            dma_display->printf("%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
            */

            /* Write out the time - place each character to fit display */
//            dma_display->setTextColor(0xF100);
            /*
            dma_display->setTextColor(
                ((timeinfo.tm_hour & 0xF8) << 8) |
                ((timeinfo.tm_min  & 0xFA) << 3) |
                ((timeinfo.tm_sec  & 0xF8))
            );
*/
#ifndef DISPLAY_COLOR
            color = colorWheel(loopcounter%256);
#else
            color = colorWheel(DISPLAY_COLOR);
#endif
            dma_display->setTextColor(color);
#ifdef SECONDS_BAR
            /* Seconds Bar */
            dma_display->drawRect(2, 19, timeinfo.tm_sec+1, 2, color);
            for (int i = 0; i <= 13; i++)
                dma_display->drawPixel(2+i*5, 20, color);
            for (int i=0; i<=5; i++)
                dma_display->drawPixel(2+i*15, 21, color);
#endif
            /* Hours and Minutes */
            dma_display->setFont(&FreeMonoBold12pt7b);
            dma_display->setTextWrap(false); // Don't wrap at end of line - will do ourselves
            dma_display->setCursor(2, 16);
            dma_display->print(timeinfo.tm_hour / 10);
            dma_display->setCursor(15, 16);
            dma_display->print(timeinfo.tm_hour % 10);
            dma_display->setCursor(35, 16);
            dma_display->print(timeinfo.tm_min / 10);
            dma_display->setCursor(48, 16);
            dma_display->print(timeinfo.tm_min % 10);
            /* Flashing colon */
            if ((loopcounter % 10) < 6) {
                dma_display->setCursor(26, 16);
                dma_display->print(":");
            }
            /* Uptime */
#ifdef SHOW_UPTIME
            dma_display->setFont();
            dma_display->setTextSize(1);     // size 1 == 8 pixels high
            dma_display->setCursor(2, 22);
            char format[20];
            uptime::calculateUptime();
            if (uptime::getDays() < 10)
                strncpy(format, "%01d %02d %02d %02d", 20);
            else if (uptime::getDays() < 100) 
                strncpy(format, "%02d %02d %02d %1d", 20);
            else if (uptime::getDays() < 10000) 
                strncpy(format, "%04d %02d %02d", 20);
            else
                strncpy(format, "%05d days", 20);

            dma_display->printf(format,
                    uptime::getDays(),
                    uptime::getHours(),
                    uptime::getMinutes(),
                    uptime::getSeconds()
            );
#endif
#ifdef SHOW_COLOR
            dma_display->setFont();
            dma_display->setTextSize(1);     // size 1 == 8 pixels high
            dma_display->setCursor(24, 22);
            dma_display->printf("%03d", loopcounter % 256);
#endif
#ifdef SHOW_DAY
            dma_display->setFont();
            dma_display->setTextSize(1);     // size 1 == 8 pixels high
            uint8_t txt_len = 6 * strlen(days[timeinfo.tm_wday]);
            uint8_t start_x = (dma_display->width() - txt_len) < 0 ? 0 : (dma_display->width() - txt_len) / 2;
            dma_display->setCursor(start_x, 23);
            dma_display->print(days[timeinfo.tm_wday]);
#endif
        } else {
            dma_display->setTextSize(1);     // size 1 == 8 pixels high
            dma_display->setTextWrap(false); // Don't wrap at end of line - will do ourselves
            dma_display->setCursor(0, 0);    // start at top left, with 8 pixel of spacing
            dma_display->printf("Wifi: %s", WiFi.status() == WL_CONNECTED ? "OK" : "Wait");
//            dma_display->setCursor(0, 9);
//            dma_display->printf(WiFi.localIP().toString().c_str());
        }
        loopcounter++;
        /* Delay 100ms taking account of time spent in loop */
        dly = (100 - (long) (millis() - start)) < 0 ? 0 : (100 - (long) (millis() - start));
        delay(dly);
//        delay(100);
    }
}