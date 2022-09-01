/**
 * @file ledstrip.c
 * @author Manu
 * @author Ollo
 * @author tobi
 *
 * creation date 2014-11-08
 *
 * Inspired by: https://github.com/omriiluz/WS2812B-LED-Driver-ChibiOS/blob/master/LEDDriver.c
 *
 */

#include "ledstripe.h"
#include <Adafruit_NeoPixel.h>

/******************************************************************************
 *                            LOCAL VARIABLES
 ******************************************************************************/
static Adafruit_NeoPixel* pPixels = NULL;

ledstripe_color ledstripe_framebuffer[LEDSTRIPE_FRAMEBUFFER_SIZE];


/******************************************************************************
 *                            GLOBAL FUNCTIONS
 ******************************************************************************/

void ledstripe_init(int pin) {
	pPixels = new Adafruit_NeoPixel(LEDSTRIPE_FRAMEBUFFER_SIZE, pin, NEO_GRB + NEO_KHZ800);
  	pPixels->begin();
  	pPixels->clear();
    for( int i = 0; i < LEDSTRIPE_FRAMEBUFFER_SIZE; i++ ) {
        pPixels->setPixelColor(i, pPixels->Color(0 /*red */, 120 /* green */, 0 /* blue */));
    }
    pPixels->show();   // make sure it is visible

}

void ledstrip_fill(uint8_t r, uint8_t g, uint8_t b) {
    pPixels->fill(pPixels->Color(r,g,b));

    Serial.printf("fill: %d-%d-%d\r\n", r, g, b);
    Serial.flush();
}

void ledstripe_update(void) {
    int i;
    for (i=0; i < LEDSTRIPE_FRAMEBUFFER_SIZE; i++) {
        pPixels->setPixelColor(i, pPixels->Color(   ledstripe_framebuffer[i].red, 
                                                    ledstripe_framebuffer[i].green, 
                                                    ledstripe_framebuffer[i].blue));
    }
}

void ledstripe_show(void) {
    pPixels->show();   // make sure it is visible
}
