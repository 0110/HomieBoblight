/**
 * @file main.cpp
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2022-08-22
 * 
 * @copyright Copyright (c) 2022
 * 
 */


/******************************************************************************
 *                                     INCLUDES
******************************************************************************/
#include <Homie.h>
#include "ledstripe.h"
#include "boblight.h"

/******************************************************************************
 *                                     DEFINES
******************************************************************************/
#define HOMIE_FIRMWARE_NAME "Boblight"

/******************************************************************************
 *                                     MACROS
******************************************************************************/

/******************************************************************************
 *                                     TYPE DEFS
******************************************************************************/

/******************************************************************************
 *                            FUNCTION PROTOTYPES
******************************************************************************/

/******************************************************************************
 *                            LOCAL VARIABLES
******************************************************************************/
bool mConfigured = false;

/******************************************************************************
 *                            LOCAL FUNCTIONS
******************************************************************************/


/******************************************************************************
 *                            GLOBAL FUNCTIONS
******************************************************************************/

void setup() {
  Serial.begin(115200);
  Homie_setFirmware(HOMIE_FIRMWARE_NAME, "1.0.0");
  
  Homie.setup();
  
  mConfigured = Homie.isConfigured();
  if (!mConfigured) {
    Serial << "Not configured" << endl;
  } else {
    ledstripe_init(D4 /* GPIO2 */);
    boblight_init(); 
  }
}

void loop() {
  Homie.loop();
  if (mConfigured) {
    boblight_loop();
    ledstripe_show();
  }
}
