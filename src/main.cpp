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
HomieNode ledNode("led", "RGB led", "color");
HomieNode tvInput("local", "USB control enabled", "switch");

bool mConfigured = false;
bool mConnected = false;
bool mSerialInput = true; /**< Serial control via USB UART */

/******************************************************************************
 *                            LOCAL FUNCTIONS
******************************************************************************/

void onHomieEvent(const HomieEvent &event)
{
  switch (event.type)
  {
  case HomieEventType::SENDING_STATISTICS:
    if (mSerialInput) {
      tvInput.setProperty("value").send("ON");
    } else {
      tvInput.setProperty("value").send("OFF");
    }
    break;
  case HomieEventType::MQTT_READY:
    mConnected = true;
  default:
    break;
  }
}

void loopHandler() {
  if (mConfigured) {
    boblight_loop();
    if (mSerialInput) {
      ledstripe_show();
    }
  }
}

bool allLedsHandler(const HomieRange& range, const String& value) {
  if (range.isRange) return false;  // only one switch is present

  int sep1 = value.indexOf(',');
  int sep2 = value.indexOf(',', sep1 + 1);
  int red = value.substring(0,sep1).toInt(); /* OpenHAB  hue (0-360°) */
  int green = value.substring(sep1 + 1, sep2).toInt(); /* OpenHAB saturation (0-100%) */
  int blue = value.substring(sep2 + 1, value.length()).toInt(); /* brightness (0-100%) */

  uint8_t r = (red * 255) / 250;
  uint8_t g = (green *255) / 250;
  uint8_t b = (blue *255) / 250;
  ledstrip_fill(r, g, b);
  if (mConnected) {
    ledNode.setProperty("ambient").send(String(value));
  }
  return true;
}

bool switchHandler(const HomieRange& range, const String& value) {
  if (range.isRange) return false;  // only one switch is present
  if (value == "off" || value == "Off" || value == "OFF" || value == "false") {
    mSerialInput = false;
    tvInput.setProperty("value").send(value);
  } else if (value == "on" || value == "On" || value == "ON" || value == "true") {
    mSerialInput = true;
    tvInput.setProperty("value").send(value);
  }
  return true;
}

/******************************************************************************
 *                            GLOBAL FUNCTIONS
******************************************************************************/

void setup() {
  Serial.begin(115200);
  Homie_setFirmware(HOMIE_FIRMWARE_NAME, "1.1.0");
  Homie.setLoopFunction(loopHandler);
  Homie.onEvent(onHomieEvent);
  Homie_setBrand("Ambilight");
  ledNode.advertise("ambient").setName("All Leds")
                            .setDatatype("color").setFormat("rgb")
                            .settable(allLedsHandler);
  tvInput.advertise("value").setName("Value")
                                      .setDatatype("Boolean")
                                      .settable(switchHandler);
  Homie.setup();
  
  mConfigured = Homie.isConfigured();
  if (!mConfigured) {
    Serial.println("Not configured");
    Serial.flush();
  } else {
    Serial.println("Start LED");
    ledstripe_init(D4 /* GPIO2 */);
    boblight_init(); 
  }
}

void loop() {
  Homie.loop();
}
