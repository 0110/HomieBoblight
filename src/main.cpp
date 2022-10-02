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

#define HOMIE_FIRMWARE_VERSION "1.5.0"


#define WORKING_INTERVAL  50 /* ms */

/* Homie */
#define HOMIE_AMBIENT                     "ambient"
#define NODE_STATISTIC_COUNT_RECEVIED     "received"
#define NODE_STATISTIC_COUNT_DUPLICATE    "duplicate"
#define NODE_STATISTIC_COUNT_VALID        "valid"
#define NODE_STATISTIC_COUNT_SUPERBRIGHT  "superbright"
#define NODE_STATISTIC_COUNT_PINGS        "adaPings"
#define NODE_STATISTIC_COUNT_SPLIT        "split"
#define NODE_STATISTIC_COUNT_CRC_ERR      "crcErr"

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
HomieNode mNodeStatistic("statistic", "Boblight Statistics", "statistic");

HomieSetting<bool> mSettingLogging("debug", "MQTT topic with debug logs");

unsigned long mLastAction = 0;
bool mConfigured = false;
bool mConnected = false;
bool mSerialInput = true; /**< Serial control via USB UART */

static long mCountRecevied    = -1;
static long mCountDuplicate   = -1;
static long mCountValid       = -1;
static long mCountSuperBright = -1;
static long mCountPings       = -1;
static long mCountSplit       = -1;
static long mCountErrorCRC    = -1;

/******************************************************************************
 *                            LOCAL FUNCTIONS
******************************************************************************/

void onHomieEvent(const HomieEvent &event)
{
  switch (event.type)
  {
  case HomieEventType::WIFI_CONNECTED:
    ledstrip_status(0, 0, 128);
    mConnected = true;
    break;
  case HomieEventType::WIFI_DISCONNECTED:
    mConnected = false;
    mSerialInput = true;
    Serial << "No Wifi" << endl;
    break;
  case HomieEventType::MQTT_READY:
    ledstrip_status(5, 0, 0);
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

  if ((mConfigured) && (aliveWasRead())) {
    /* Update all statistics about received Boblight communication */
    if (mCountRecevied    != getCountRecevied())
    {
      mCountRecevied    = getCountRecevied();
      mNodeStatistic.setProperty(NODE_STATISTIC_COUNT_RECEVIED)
                      .send(String(mCountRecevied));
    }

    if (mCountDuplicate   != getCountDuplicate()) {
      mCountDuplicate   = getCountDuplicate();
      mNodeStatistic.setProperty(NODE_STATISTIC_COUNT_DUPLICATE)
                      .send(String(mCountDuplicate));
    }

    if (mCountValid       != getCountValid()) {
      mCountValid       = getCountValid();
      mNodeStatistic.setProperty(NODE_STATISTIC_COUNT_VALID)
                      .send(String(mCountValid));
    }

    if (mCountSuperBright != getCountSuperBright()) {
      mCountSuperBright = getCountSuperBright();
      mNodeStatistic.setProperty(NODE_STATISTIC_COUNT_SUPERBRIGHT)
                      .send(String(mCountSuperBright));
    }

    if (mCountErrorCRC != getCountErrorCRC()) {
      mCountErrorCRC = getCountErrorCRC();
      mNodeStatistic.setProperty(NODE_STATISTIC_COUNT_CRC_ERR)
                      .send(String(mCountErrorCRC));

    }
    
    if (mCountSplit != getCountSplit()) {
      mCountSplit = getCountSplit();
      mNodeStatistic.setProperty(NODE_STATISTIC_COUNT_SPLIT)
                      .send(String(mCountSplit));

    }

    if (mCountPings != getCountPings()) {
      mCountPings = getCountPings();
      mNodeStatistic.setProperty(NODE_STATISTIC_COUNT_PINGS)
                      .send(String(mCountPings));
    }
  }

  // Feed the dog -> ESP stay alive
  ESP.wdtFeed();
}

bool allLedsHandler(const HomieRange& range, const String& value) {
  if (range.isRange) return false;  // only one switch is present

  int sep1 = value.indexOf(',');
  int sep2 = value.indexOf(',', sep1 + 1);
  if ((sep1 > 0) && (sep2 > 0) && (mSerialInput == 0)) {
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
    mqttlog(LOG_LEVEL_DEBUG, String(String("Serial=") + String(mSerialInput) + String(" Color: " + value)));
    return false;
  }
}

bool switchHandler(const HomieRange& range, const String& value) {
  if (range.isRange) return false;  // only one switch is present
  if (value == "off" || value == "Off" || value == "OFF" || value == "false") {
    mSerialInput = false;
    mNodeTVsource.setProperty("value").send(value);
    mqttlog(LOG_LEVEL_DEBUG, String("Serial input deactivated"));
  } else if (value == "on" || value == "On" || value == "ON" || value == "true") {
    mSerialInput = true;
    mNodeTVsource.setProperty("value").send(value);
    mqttlog(LOG_LEVEL_DEBUG, String("Serial input activated"));
  } else {
    return false;
  }
  return true;
}

/******************************************************************************
 *                            GLOBAL FUNCTIONS
******************************************************************************/

void setup() {
  Serial.begin(115200);
  Homie_setFirmware(HOMIE_FIRMWARE_NAME, HOMIE_FIRMWARE_VERSION);
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

  mNodeStatistic.advertise(NODE_STATISTIC_COUNT_RECEVIED).
                  setName(NODE_STATISTIC_COUNT_RECEVIED).
                  setDatatype("Integer");
  mNodeStatistic.advertise(NODE_STATISTIC_COUNT_DUPLICATE).
                  setName(NODE_STATISTIC_COUNT_DUPLICATE).
                  setDatatype("Integer");
  mNodeStatistic.advertise(NODE_STATISTIC_COUNT_VALID).
                  setName(NODE_STATISTIC_COUNT_VALID).
                  setDatatype("Integer");
  mNodeStatistic.advertise(NODE_STATISTIC_COUNT_SUPERBRIGHT).
                  setName(NODE_STATISTIC_COUNT_SUPERBRIGHT).
                  setDatatype("Integer");
  mNodeStatistic.advertise(NODE_STATISTIC_COUNT_PINGS).
                  setName(NODE_STATISTIC_COUNT_PINGS).
                  setDatatype("Integer");
  mNodeStatistic.advertise(NODE_STATISTIC_COUNT_SPLIT).
                  setName(NODE_STATISTIC_COUNT_SPLIT).
                  setDatatype("Integer");
  mNodeStatistic.advertise(NODE_STATISTIC_COUNT_CRC_ERR).
                  setName(NODE_STATISTIC_COUNT_CRC_ERR).
                  setDatatype("Integer");

  ledstripe_init(D1 /* GPIO5 */);
  mConfigured = Homie.isConfigured();
  if (!mConfigured) {
    Serial.println("Not configured");
    Serial.flush();
  } else {
    boblight_init();
    delay(100); /* wait 100ms */
    Serial.println("Init done");
    Serial.flush();
  }
}

void loop() {
  Homie.loop();
  /* Update the LEDs */
  if (mConfigured) {
    if (mSerialInput) {
      if (boblight_loop()) {
        ledstripe_update();
        ledstripe_show();
      }
    } else {
      /* Color was already set in allLedsHandler function */
    }
  } else {
    if ( ((millis() - mLastAction) >= (WORKING_INTERVAL)) ) {
      ledstripe_toggle(60, 0, 0); /* Red indicates not configured */
    }
    mLastAction = millis();
  }
}
