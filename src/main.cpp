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
#include "MQTTUtils.h"

/******************************************************************************
 *                                     DEFINES
******************************************************************************/
#define HOMIE_FIRMWARE_NAME "Boblight"

#define HOMIE_AMBIENT "ambient"

#define WORKING_INTERVAL  50 /* ms */

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


unsigned long mLastAction = 0;
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
  case HomieEventType::WIFI_CONNECTED:
    mConnected = true;
    break;
  case HomieEventType::WIFI_DISCONNECTED:
    mConnected = false;
    mSerialInput = true;
    Serial << "No Wifi" << endl;
    break;
  case HomieEventType::MQTT_READY:
    mqttSetAlive();
    if (mSerialInput) {
      mNodeTVsource.setProperty("value").send("ON");
    } else {
      mNodeTVsource.setProperty("value").send("OFF");
    }
    break;
  default:
    break;
  }
}

void loopHandler() {
  if ( ((millis() - mLastAction) >= (WORKING_INTERVAL)) ||
      (mLastAction == 0) ) {
    if (mConfigured) {
      if (mSerialInput) {
        boblight_loop();
        ledstripe_update();
      }
      ledstripe_show();
    } else {
      ledstripe_toggle(60, 0, 0); /* Red indicates not configured */
    }
    mLastAction = millis();
  }

  // Feed the dog -> ESP stay alive
  ESP.wdtFeed();
}

bool allLedsHandler(const HomieRange& range, const String& value) {
  if (range.isRange) return false;  // only one switch is present

  int sep1 = value.indexOf(',');
  int sep2 = value.indexOf(',', sep1 + 1);
  if ((sep1 > 0) && (sep2 > 0) && mSerialInput) {
    int red = value.substring(0,sep1).toInt(); 
    int green = value.substring(sep1 + 1, sep2).toInt(); 
    int blue = value.substring(sep2 + 1, value.length()).toInt();

    uint8_t r = (red * 255) / 250;
    uint8_t g = (green *255) / 250;
    uint8_t b = (blue *255) / 250;
    ledstrip_fill(r, g, b);

    if (mConnected) {
      mNodeLed.setProperty(HOMIE_AMBIENT).send(String(value));
    }
    return true;
  } else {
    log(LOG_LEVEL_DEBUG, String(String(mSerialInput) + String(" Color: " + value)));
    return false;
  }
}

bool switchHandler(const HomieRange& range, const String& value) {
  if (range.isRange) return false;  // only one switch is present
  if (value == "off" || value == "Off" || value == "OFF" || value == "false") {
    mSerialInput = false;
    mNodeTVsource.setProperty("value").send(value);
    log(LOG_LEVEL_DEBUG, String("Serial input deactivated"));
  } else if (value == "on" || value == "On" || value == "ON" || value == "true") {
    mSerialInput = true;
    mNodeTVsource.setProperty("value").send(value);
    log(LOG_LEVEL_DEBUG, String("Serial input activated"));
  }
  return true;
}

/******************************************************************************
 *                            GLOBAL FUNCTIONS
******************************************************************************/

void setup() {
  Serial.begin(115200);
  Homie_setFirmware(HOMIE_FIRMWARE_NAME, "1.1.0");
  Homie.disableLogging();
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
    ledstripe_init(D1 /* GPIO5 */);
    boblight_init(); 
  }
}

void loop() {
  Homie.loop();
}
