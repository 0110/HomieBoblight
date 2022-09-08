#include "MQTTUtils.h"


bool volatile mAliveWasRead = false;

void mqttlog(int level, String message)
{
  String buffer;
  StaticJsonDocument<200> doc;
  // Read the current time
  time_t now; // this is the epoch
  tm tm;      // the structure tm holds time information in a more convient way
  doc["level"] = level;
  doc["message"] = message;
  time(&now);
  localtime_r(&now, &tm);
  if (tm.tm_year > (2021 - 1970)) { /* Only add the time, if we have at least 2021 */
    doc["time"] =  String(String(1900 + tm.tm_year) + "-" + String(tm.tm_mon + 1) + "-" + String(tm.tm_mday) +
              " " + String(tm.tm_hour) + ":" + String(tm.tm_min) + ":" + String(tm.tm_sec));
  }

  serializeJson(doc, buffer);
  if (mAliveWasRead)
  {
    getTopic(LOG_TOPIC, logTopic)
        Homie.getMqttClient()
            .subscribe(logTopic, 2);

    Homie.getMqttClient().publish(logTopic, 2, false, buffer.c_str());
    delete logTopic;
  }
}

bool aliveWasRead(){
    return mAliveWasRead;
}

void mqttSetAlive() {
  mAliveWasRead = true;
}
