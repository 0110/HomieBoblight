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
Adafruit_NeoPixel* pPixels = NULL;

ledstripe_color ledstripe_framebuffer[LEDSTRIPE_FRAMEBUFFER_SIZE];


/******************************************************************************
 *                            GLOBAL VARIABLES
 ******************************************************************************/

void ledstripe_init(int pin) {
	pPixels = new Adafruit_NeoPixel(LEDSTRIPE_FRAMEBUFFER_SIZE, pin, NEO_GRB + NEO_KHZ800);
  	pPixels->begin();
  	pPixels->clear();
}

void ledstrip_fill(uint8_t r, uint8_t g, uint8_t b) {
    uint32_t c = pPixels->Color(r,g,b);
    pPixels->fill(c);
    pPixels->show();   // make sure it is visible
}

void ledstripe_show(void) {
	//FIXME
}
