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

#define HOMIE_AMBIENT "ambient"

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
HomieNode mNodeLed("led", "Light", "led");
HomieNode mNodeTVsource("control", "USB control enabled", "switch");

HomieSetting<bool> mSettingLogging("debug", "MQTT topic with debug logs");


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
      mNodeTVsource.setProperty("value").send("ON");
    } else {
      mNodeTVsource.setProperty("value").send("OFF");
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
  // Feed the dog -> ESP stay alive
  ESP.wdtFeed();
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
    mNodeLed.setProperty(HOMIE_AMBIENT).send(String(value));
  }
  return true;
}

bool switchHandler(const HomieRange& range, const String& value) {
  if (range.isRange) return false;  // only one switch is present
  if (value == "off" || value == "Off" || value == "OFF" || value == "false") {
    mSerialInput = false;
    mNodeTVsource.setProperty("value").send(value);
  } else if (value == "on" || value == "On" || value == "ON" || value == "true") {
    mSerialInput = true;
    mNodeTVsource.setProperty("value").send(value);
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
                            
  Homie.setup();
  
  mSettingLogging.setDefaultValue(false).setValidator([] (int candidate) {
    return true;
  });

  mNodeTVsource.advertise("value").setName("Value")
                                      .setDatatype("Boolean")
                                      .settable(switchHandler);
  mNodeLed.advertise(HOMIE_AMBIENT).setName("RGBType")
                            .setDatatype("color").settable(allLedsHandler);


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
