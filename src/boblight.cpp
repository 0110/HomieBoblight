/*
 * boblight.c
 *
 *  Created on: Apr 4, 2015
 *  Imported 2022-08-22
 *      Author: c3ma
 */

#include "crc32.h"
#include "ledstripe.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/******************************************************************************
 * DEFINES
 ******************************************************************************/

#define	STARTOFFSET				2
#define HEX_SIZE				2
#define TEXTLINE_MAX_LENGTH 	1024

#define COLOR_RESET				0
#define COLOR_RED				1
#define COLOR_GREEN				2
#define COLOR_BLUE				3

#define BOBLIGHT_MAILBOX_SIZE	1000
#define BOBLIGHT_MAILBOX_DECR	50		/**< Amount of entries that are removed by one step */
#define BOBLIGHT_MAILBOX_DECR_BORDER 100 /**< When Mailbox increased this amount,  BOBLIGHT_MAILBOX_DECR is used for decrementing */

#define LOGGING_FACTOR		2

/** Constants for the dynamic dimming */
#define FACTOR_DEFAULT	1000	/**< No change, so it is set to 1000 promill */
#define FACTOR_DECREASE_STEP	1	/**< Decrease the amount in each cycle by 1 promill */
#define FACTOR_MINIMUM		330	/**< As one color can be easily at full brightness, 330 promill is the minimum */
#define FACTOR_INCREASE_STEP	2	/**< Increasing amount in each cycle */

#define FACTOR_DECREASE_SAME_STEP    3   /**< Decrease at same color the amount in each cycle by 1 promill */
#define ALLOWED_SAME_COLOR      100  /**< cycles, the same color can be shown */

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

/******************************************************************************
 * LOCAL VARIABLES for this module
 ******************************************************************************/

static int channelSize = 0;
static int ledOffset = 0;
static int startFound = 0;

static int colorPosition=0;
static int dynamicColorFactor= FACTOR_DEFAULT;

static ledstripe_color ledstripe_inputbuffer[LEDSTRIPE_FRAMEBUFFER_SIZE]; /**< Without any modification */

/** DEBUG */
static uint32_t meanSum = 0;
static uint32_t meanRed = 0;
static uint32_t meanGreen = 0;
static uint32_t meanBlue = 0;

static uint32_t oldCRCvalue         = 0U;
static uint32_t sameColorCounter    = 0U;

static char textbuffer[TEXTLINE_MAX_LENGTH];

/******************************************************************************
 * LOCAL FUNCTIONS for this module
 ******************************************************************************/

/** Secure, that the leds are not oo bright for a long time */
static void calculateDynamicDim( void )
{
	uint32_t sumRed=0;
	uint32_t sumGreen=0;
	uint32_t sumBlue=0;
	uint32_t crc = 0;
	int i;
	for(i=0; i < ledOffset; i++)
	{
		sumRed += ledstripe_framebuffer[i].red;
		sumGreen += ledstripe_framebuffer[i].green;
		sumBlue += ledstripe_framebuffer[i].blue;
	}

	crc = crc32((uint8_t *) &ledstripe_inputbuffer, sizeof(ledstripe_inputbuffer));

	/* Get the mean brighntess per color */
	meanRed =   (sumRed / ledOffset);
	meanGreen = (sumGreen / ledOffset);
	meanBlue =  (sumBlue / ledOffset);
	meanSum = (meanRed + meanGreen + meanBlue);
	/* Handle too bright LEDs */
	if (meanSum > 255)
	{
		if (dynamicColorFactor > FACTOR_MINIMUM)
		{
			/* Color is too bright for longer periods */
			dynamicColorFactor -= FACTOR_DECREASE_STEP;
		}
	}
	else if (crc == oldCRCvalue) {
	    if (sameColorCounter > ALLOWED_SAME_COLOR) {
	        /* Same color for longer period... dim the lights down */
	        if (dynamicColorFactor > FACTOR_DECREASE_SAME_STEP) {
	            dynamicColorFactor -= FACTOR_DECREASE_SAME_STEP;
	        } else if (dynamicColorFactor <= FACTOR_DECREASE_SAME_STEP){
	            dynamicColorFactor = 0;
	        }

            /* reset counter */
            sameColorCounter = 1U;
	    } else {
	        sameColorCounter++;
	    }
	}
	else
	{
        if (dynamicColorFactor < FACTOR_MINIMUM) {
            /* reset */
            sameColorCounter = 0U;
            crc = 0U;
            dynamicColorFactor = FACTOR_MINIMUM;
        } else if (dynamicColorFactor < FACTOR_DEFAULT)
        /* Dark values are no problem for long terms */
		{
			dynamicColorFactor += FACTOR_INCREASE_STEP;
		}
		else
		{
			dynamicColorFactor = FACTOR_DEFAULT;
		}
	}

	/* remember the CRC value for the next cycle */
	oldCRCvalue = crc;
}

static int readDirectWS2812cmd(char *textbuffer)
{
	int i=0;
	int length = Serial.read(textbuffer, TEXTLINE_MAX_LENGTH);
	if(length > 0)
	{
		for(i=0; i < length; i++)
		{
			/** Handle Frame beginning */
			if (textbuffer[i] == 'A' && textbuffer[i+1] == 'd' && textbuffer[i+2] == 'a')
			{
				calculateDynamicDim();
				channelSize = textbuffer[i+3] * 256 + textbuffer[i+4];
				i=i+5;
				ledOffset=0;
				colorPosition=COLOR_RED;
				startFound=TRUE;
			}
			else if (startFound == TRUE)
			{
				/* Update the LED information */
				if (ledOffset < LEDSTRIPE_FRAMEBUFFER_SIZE)
				{
					switch(colorPosition)
					{
					case COLOR_RED:
						ledstripe_framebuffer[ledOffset].red = 		(uint8_t) ((textbuffer[i] * dynamicColorFactor) / FACTOR_DEFAULT);
						ledstripe_inputbuffer[ledOffset].red =      (uint8_t) (textbuffer[i]);
						break;
					case COLOR_GREEN:
						ledstripe_framebuffer[ledOffset].green = 	(uint8_t) ((textbuffer[i] * dynamicColorFactor) / FACTOR_DEFAULT);
						ledstripe_inputbuffer[ledOffset].green =      (uint8_t) (textbuffer[i]);
						break;
					case COLOR_BLUE:
						ledstripe_framebuffer[ledOffset].blue = 	(uint8_t) ((textbuffer[i] * dynamicColorFactor) / FACTOR_DEFAULT);
						ledstripe_inputbuffer[ledOffset].blue =      (uint8_t) (textbuffer[i]);
						/* Reset for the next LED */
						colorPosition = COLOR_RESET;
						ledOffset++;
						break;
					}
					colorPosition++;
				}
			}
		}
		return TRUE;
	}
	else
	{
		return FALSE; /* Nothing found */
	}
}

/******************************************************************************
 * GLOBAL FUNCTIONS for this module
 ******************************************************************************/

void boblight_init(void)
{
	/* Say hello to the host */
	Serial.print("Ada\n");
	memset(textbuffer, 0, TEXTLINE_MAX_LENGTH);
}

int boblight_loop(void)
{
	long time = millis();
	//read serial input
	readDirectWS2812cmd(textbuffer);

	/* Send ACK to host each second */
	if ((time + 1000) < millis())
	{
		Serial.print("Ada\n");
		time = millis();
	}

	return TRUE;
}