/* @file ledstripe.h
 * @brief Interface to use ledstripe
 * @author Ollo
 *
 * @defgroup LEDSTRIPE
 */

#ifndef _LEDSTRIPE_H_
#define _LEDSTRIPE_H_

#include <Adafruit_NeoPixel.h>
#include "stdint.h"

#define LEDSTRIPE_COLOR_MAXVALUE  255

//enter number of LEDs here!
#define LEDSTRIPE_FRAMEBUFFER_SIZE 240
#define LEDSTRIPE_TOGGLE_LENGTH		10

#define	RGB_COLOR_WIDTH				3 /**< Three bytes are needed to describe one LED */

//Size of Ring-Buffer
#define LEDSTRIPE_PWM_BUFFER_SIZE 192

typedef struct {
	uint8_t red;
	uint8_t green;
	uint8_t blue;
} ledstripe_color;

/** @addtogroup LEDSTRIPE */
/*@{*/

#ifdef __cplusplus
extern "C"
{
#endif

extern ledstripe_color ledstripe_framebuffer[LEDSTRIPE_FRAMEBUFFER_SIZE];

/** @fn void ledstripe_init(void)
 * @brief Initialization of the Timer/PWM and DMA
 * @param[in] pin       The pin to use, e.g.:  GPIO2
 * The PIN and the port number must match! This is NOT checked internally.
 */
void ledstripe_init(int pin);

/**
 * @brief Write the currently set LED values to the hardware
 * 
 */
void ledstripe_show(void);

/**
 * @brief convert buffer to LED hardware 
 * Input is defined in @see ledstripe_framebuffer
 */
void ledstripe_update(void);

void ledstrip_fill(uint8_t r, uint8_t g, uint8_t b);

/**
 * @brief Update a subset of LEDs (defined by @see LEDSTRIPE_TOGGLE_LENGTH)
 * with the color, defined by all three arguments
 * @param r 	red		(0-255)
 * @param g 	green	(0-255)
 * @param b 	blue	(0-255)
 */
void ledstrip_status(uint8_t r, uint8_t g, uint8_t b);

void ledstripe_toggle(uint8_t red, uint8_t green, uint8_t blue);

#ifdef __cplusplus
}
#endif

/*@}*/
#endif /*_LEDSTRIPE_H_*/
