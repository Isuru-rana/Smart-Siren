void mqttsend(bool type, int message, bool state) {
  if (type) {
    const size_t capacity = JSON_OBJECT_SIZE(1);
    DynamicJsonDocument doc(capacity);
    doc["status"] = "Online";
    String jsonStr;
    serializeJson(doc, jsonStr);
    client.publish(heartBeat, jsonStr.c_str(), 1);
  }

  else {
    if (message == 1) {  //Siren State
      const size_t capacity = JSON_OBJECT_SIZE(1);
      DynamicJsonDocument doc(capacity);
      //doc["device"] = "siren";
      if (state) {
        doc["state"] = 1;
      } else {
        doc["state"] = 0;
      }
      String jsonStr;
      serializeJson(doc, jsonStr);
      client.publish(sta_siren, jsonStr.c_str(), 1);
    } else if (message == 2) {  //Lights State
      const size_t capacity = JSON_OBJECT_SIZE(1);
      DynamicJsonDocument doc(capacity);
      //doc["device"] = "lights";
      if (state) {
        doc["state"] = 1;
      } else {
        doc["state"] = 0;
      }
      String jsonStr;
      serializeJson(doc, jsonStr);
      client.publish(sta_lights, jsonStr.c_str(), 1);
    } else if (message == 3) {  //Motion Detect
      const size_t capacity = JSON_OBJECT_SIZE(1);
      DynamicJsonDocument doc(capacity);
      //doc["device"] = "lights";
      if (state) {
        doc["state"] = state;
      } else {
        doc["state"] = state;
      }
      String jsonStr;
      serializeJson(doc, jsonStr);
      client.publish(motionDetectsta, jsonStr.c_str());
    } else if (message == 4) {  //lights pattern
      const size_t capacity = JSON_OBJECT_SIZE(1);
      DynamicJsonDocument doc(capacity);
      doc["pattern"] = light_pattern;
      String jsonStr;
      serializeJson(doc, jsonStr);
      client.publish(sta_lignts_pattern, jsonStr.c_str(), 1);
    } else if (message == 5) {  //SOS state
      const size_t capacity = JSON_OBJECT_SIZE(1);
      DynamicJsonDocument doc(capacity);
      doc["state"] = sosState;
      String jsonStr;
      serializeJson(doc, jsonStr);
      client.publish(sos_sta, jsonStr.c_str());
    } else if (message == 6) {  //Siren Timer
      const size_t capacity = JSON_OBJECT_SIZE(1);
      DynamicJsonDocument doc(capacity);
      doc["time"] = sirenTimer;
      String jsonStr;
      serializeJson(doc, jsonStr);
      client.publish(sta_siren_timer, jsonStr.c_str());
    } else if (message == 9) {  //RSSI state
      const size_t capacity = JSON_OBJECT_SIZE(1);
      DynamicJsonDocument doc(capacity);
      doc["strength"] = rssi;
      String jsonStr;
      serializeJson(doc, jsonStr);
      client.publish(sta_rssi, jsonStr.c_str());
    }
  }
}


void callBack(char* topic, byte* payload, unsigned int length) {

  char message[length + 1];
  memcpy(message, payload, length);
  message[length] = '\0';

  DynamicJsonDocument jsonBuffer(512);
  DeserializationError error = deserializeJson(jsonBuffer, message);

  if (error) {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.f_str());
    return;
  }

  if (String(topic) == HA_Boot) {
    if (!jsonBuffer.containsKey("status")) {
      return;
    }
    const char* state = jsonBuffer["status"];
    if (strcmp(state, "online") == 0) {
      mqttsend(1, 0, 0);
    }
  } else if (String(topic) == cmd_siren) {
    if (!jsonBuffer.containsKey("state")) {
      return;
    }
    bool state = jsonBuffer["state"];

    if (state == 1) {
      siren_state = 1;
      mqttsend(0, 1, 1);
    } else if (state == 0) {
      siren_state = 0;
      mqttsend(0, 1, 0);
    }
  } else if (String(topic) == cmd_lights) {
    if (!jsonBuffer.containsKey("state")) {
      return;
    }
    bool state = jsonBuffer["state"];

    if (state == 1) {
      light_state = 1;
      mqttsend(0, 2, 1);
    } else if (state == 0) {
      light_state = 0;
      mqttsend(0, 2, 0);
    }
  } else if (String(topic) == cmd_lignts_pattern) {
    if (!jsonBuffer.containsKey("pattern")) {
      return;
    }
    int pattern = jsonBuffer["pattern"];
    light_pattern = pattern;
    mqttsend(0, 4, 0);
  } else if (String(topic) == sos_cmd) {
    if (!jsonBuffer.containsKey("state")) {
      return;
    }
    bool state = jsonBuffer["state"];
    if (state == 1) {
      sosState = 1;
      mqttsend(0, 5, 1);
    } else if (state == 0) {
      sosState = 0;
      mqttsend(0, 5, 0);
    }
  } else if (String(topic) == cmd_siren_timer) {
    if (!jsonBuffer.containsKey("timer")) {
      return;
    }
    int time = jsonBuffer["timer"];
    if (time < 1800000 && time > 0) {
      sirenTimer = time;
      mqttsend(0, 6, 0);
    }
  }
}
