#include <Arduino.h>
#include <WiFi.h>
#include <uptime.h>


#define COLOR_VFD       225
#define COLOR_RED       84
#define DISPLAY_COLOR   84     // Define a singe colour from the color wheel for the display
//#define SHOW_UPTIME   1
//#define SHOW_COLOR    1
#define SHOW_DAY        1
#define SECONDS_BAR     1
#define LOOP_MS         40      // 25Hz

#include <etask_rgbpanel_clock.h>
#include <etask_wifi.h>

#include <Fonts/FreeMonoBold12pt7b.h>

#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>

MatrixPanel_I2S_DMA *dma_display = nullptr;

PROGMEM static char days[7][10] = {"Saturday", "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday"};

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
// From: https://gist.github.com/davidegironi/3144efdc6d67e5df55438cc3cba613c8
uint16_t colorWheel(uint8_t pos, uint8_t dim = 255) {

    dim = dim < 1 ? 1 : dim;
  if(pos < 85) {
    return dma_display->color565((pos * 3 * dim) / 255, (255 - pos * 3) * dim / 255, 0);
  } else if(pos < 170) {
    pos -= 85;
    return dma_display->color565((255 - pos * 3) * dim / 255, 0, (pos * 3 * dim) / 255);
  } else {
    pos -= 170;
    return dma_display->color565(0, (pos * 3 * dim) / 255, (255 - pos * 3) * dim / 255);
  }
}

/* Gets a time value based on the real-time, not time since start */
int64_t getTimeMS() {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (int16_t) (tv.tv_usec / 1000LL);
	return (tv.tv_sec * 1000LL + (tv.tv_usec / 1000LL));
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
    dma_display->setBrightness8(128); // 0-255
    dma_display->clearScreen();
    dma_display->flipDMABuffer();

    // Declare loop variables outside the loop
    time_t thetime              = 0;
    unsigned long start         = 0;
    unsigned long end           = 0;
    unsigned long looptime      = 0;
    unsigned long loopcounter   = 0;
    long dly                    = 0;
    uint16_t color              = 0xF800;


    /* Run forever - Required for FreeRTOS task */
    for (;;)
    {
        start = millis();
        thetime = UK.toLocal(now());

        // Set up the display for this loop
        dma_display->flipDMABuffer();
        dma_display->clearScreen();

        // Don't display the time until we have an update from NTP
        if (year(thetime) > 2020)
        {

#ifndef DISPLAY_COLOR
            color = colorWheel(loopcounter%256);
#else
            color = colorWheel(DISPLAY_COLOR);
#endif
            dma_display->setTextColor(color);
            dma_display->setFont(&FreeMonoBold12pt7b);
            dma_display->setTextSize(1);     // size 1 == 8 pixels high
            dma_display->setTextWrap(false); // Don't wrap at end of line - will do ourselves

            // Flash the central colon every second
            if ((second(thetime) % 2) == 0) {
                dma_display->setCursor(26, 16);
                dma_display->print(":");
            }

            // Hours and Minutes
            dma_display->setCursor(2, 16);
            dma_display->print(hour(thetime) / 10);
            dma_display->setCursor(15, 16);
            dma_display->print(hour(thetime) % 10);
            dma_display->setCursor(35, 16);
            dma_display->print(minute(thetime) / 10);
            dma_display->setCursor(48, 16);
            dma_display->print(minute(thetime) % 10);

#ifdef SECONDS_BAR
            /* Seconds Bar */
            dma_display->drawRect(2, 19, second(thetime) + 1, 2, color);
            for (int i = 0; i <= 13; i++)
                dma_display->drawPixel(2+i*5, 20, color);
            for (int i=0; i<=5; i++)
                dma_display->drawPixel(2+i*15, 21, color);
#endif

#ifdef SHOW_DAY
            dma_display->setFont();         // Reset to default font (8x6)
            dma_display->setTextSize(1);     // size 1 == 8 pixels high
            uint8_t txt_len = 6 * strlen(days[weekday(thetime)]);
            uint8_t start_x = (dma_display->width() - txt_len) < 0 ? 0 : (dma_display->width() - txt_len) / 2;
            dma_display->setCursor(start_x, 23);
            dma_display->print(days[weekday(thetime)]);
#endif

#ifdef SHOW_UPTIME
            /* Uptime */
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

        } else {
            // We don't have a good NTP time yet, so show wifi status
            dma_display->setFont();         // Reset to default font (8x6)
            dma_display->setTextSize(1);     // size 1 == 8 pixels high
            dma_display->setTextWrap(false); // Don't wrap at end of line - will do ourselves
            dma_display->setCursor(0, 0);    // start at top left, with 8 pixel of spacing
            dma_display->printf("Wifi %s", WiFi.status() == WL_CONNECTED ? "OK" : "Init");
//            dma_display->setCursor(0, 9);
//            dma_display->printf(WiFi.localIP().toString().c_str());
        }

        // End the loop with an appropriate delay to meet the desired refresh rate
        loopcounter++;
        end = millis();
        looptime = end - start;
        dly = LOOP_MS - looptime;
        dly = dly < 0 ? 0 : dly;
        delay(dly);
    }
}