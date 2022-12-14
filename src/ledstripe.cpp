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

static int mToogleState = 0;

/******************************************************************************
 *                            GLOBAL FUNCTIONS
 ******************************************************************************/

void ledstripe_init(int pin) {
	pPixels = new Adafruit_NeoPixel(LEDSTRIPE_FRAMEBUFFER_SIZE, pin, NEO_GRB + NEO_KHZ800);
  	pPixels->begin();
  	pPixels->clear();
    for( int i = 0; i < LEDSTRIPE_TOGGLE_LENGTH; i++ ) {
        pPixels->setPixelColor(i, pPixels->Color(STATIC_COLOR_INIT));
    }
    pPixels->show();   // make sure it is visible

}

void ledstripe_toggle(uint8_t red, uint8_t green, uint8_t blue) {
    pPixels->clear();
    if (mToogleState) {
        for( int i = 0; i < LEDSTRIPE_TOGGLE_LENGTH; i++ ) {
            pPixels->setPixelColor(i, pPixels->Color(red, green, blue));
        }
        mToogleState = 0;
    } else {
        mToogleState = 1;
    }

    pPixels->show();   // make sure it is visible
}

void ledstrip_fill(uint8_t r, uint8_t g, uint8_t b) {
    pPixels->fill(pPixels->Color(r,g,b));
    pPixels->show();   // make sure it is visible
}

void ledstrip_status(uint8_t r, uint8_t g, uint8_t b) {
  	pPixels->clear();
    for( int i = 0; i < LEDSTRIPE_TOGGLE_LENGTH; i++ ) {
        pPixels->setPixelColor(i, pPixels->Color(r /*red */, g /* green */, b /* blue */));
    }
    pPixels->show();   // make sure it is visible
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
