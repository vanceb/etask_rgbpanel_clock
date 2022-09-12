#include <Arduino.h>
#include <WiFi.h>

#include <etask_rgbpanel_clock.h>

#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>

MatrixPanel_I2S_DMA *dma_display = nullptr;

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
    dma_display->setBrightness8(90); // 0-255
    dma_display->clearScreen();
    dma_display->flipDMABuffer();

    /* Run forever - Required for FreeRTOS task */
    for (;;)
    {
        struct tm timeinfo;

        getLocalTime( &timeinfo );

        dma_display->flipDMABuffer();
        dma_display->clearScreen();
        // Valid only if year > 2000. 
        // You can get from timeinfo : tm_year, tm_mon, tm_mday, tm_hour, tm_min, tm_sec
        if (timeinfo.tm_year > 100 )
        {
            dma_display->setTextSize(2);     // size 1 == 8 pixels high
            dma_display->setTextWrap(false); // Don't wrap at end of line - will do ourselves
            dma_display->setCursor(3, 9);    // start at top left, with 8 pixel of spacing
            dma_display->printf("%02d:%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
            delay(100);
        } else {
            dma_display->setTextSize(1);     // size 1 == 8 pixels high
            dma_display->setTextWrap(false); // Don't wrap at end of line - will do ourselves
            dma_display->setCursor(0, 0);    // start at top left, with 8 pixel of spacing
            dma_display->printf("Wifi: %s", WiFi.status() == WL_CONNECTED ? "OK" : "Wait");
            dma_display->setCursor(0, 9);
            dma_display->printf(WiFi.localIP().toString().c_str());
        }
    }
}