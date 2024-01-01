void callBack(char* topic, byte* payload, unsigned int length) {

  String payloadStr = "";
  String topicStr = topic;

  for (int i = 0; i < length; i++) {
    payloadStr += (char)payload[i];
  }

  // if check connection message is recived from HA os,
  if (topicStr.equals(check_connection_rec)) {
    if (payloadStr.equals("online")) {
      Timer3.detach();
      if (node_mode) {  // set node state to HA mode again if on Indipendent mode
        node_mode = false;
        if (EEPROM.read(debug_mode_Address)) {
          client.publish("testing code", "response recived. turning off indipendent mode");
        }
      }
    }
  }
  if (topicStr.equals(debug_mode_listn)) {
    if (payloadStr.equals("Turn ON")) {
      EEPROM.write(debug_mode_Address, 1);
      EEPROM.commit();
      client.publish(debug_mode_sta, "Debug, ON!");
      debug_mode_state = true;
      client.publish("testing code", "Debug mode is TURN ON");
    } else if (payloadStr.equals("Turn OFF")) {
      EEPROM.write(debug_mode_Address, 0);
      EEPROM.commit();
      client.publish(debug_mode_sta, "Debug, OFF!");
      debug_mode_state = false;
      client.publish("testing code", "Debug mode is TURN OFF");
    }
  }
  if (topicStr.equals(debug_mode_sta)) {
    if (payloadStr.equals("????")) {
      if (debug_mode_state == true) {
        client.publish(debug_mode_sta, "Debug, ON");
      } else if (debug_mode_state == false) {
        client.publish(debug_mode_sta, "Debug, OFF");
      }
    }
  }

  if (topicStr.equals(SirenTopic_listn)) {
    if (payloadStr.equals("Turn ON")) {
      if (node_system_state == true) {
        digitalWrite(siren, HIGH);
        SirenON_OFF = true;
        Timer2.attach(Siren_on_time_in_sec, sirenOfftime);
        client.publish(SirenTopic_send, "Siren, ON!");
        if (EEPROM.read(debug_mode_Address)) {
          client.publish("testing code", "Siren Turn On with manual mqtt");
        }
      } else {
        if (EEPROM.read(debug_mode_Address)) {
          client.publish("testing code", "Siren ON restricted due to system deactivated");
        }
      }
    } else if (payloadStr.equals("Turn OFF")) {
      //if (node_system_state == true) {
      digitalWrite(siren, LOW);
      SirenON_OFF = false;
      Timer2.detach();
      client.publish(SirenTopic_send, "Siren, OFF!");
      if (EEPROM.read(debug_mode_Address)) {
        client.publish("testing code", "Siren Turn Off with manual mqtt");
      }
      //}
      /*
      else {
        if (EEPROM.read(debug_mode_Address)) {
          client.publish("testing code", "Siren already in off due to system deactivated");
        }
      }
      */
    }
  }
  if (topicStr.equals(SirenTopic_send)) {
    if (payloadStr.equals("????")) {
      if (SirenON_OFF == true) {
        client.publish(SirenTopic_send, "Siren, ON");
      } else if (SirenON_OFF == false) {
        client.publish(SirenTopic_send, "Siren, OFF");
      }
    }
  }

  if (topicStr.equals(nodeStateSetManual_Listn)) {  //  set/floor3/LR/node/mode
    if (payloadStr.equals("Turn ON")) {
      //node_mode = true;
      autoTrigger_state = true;
      client.publish(nodeStateSetManual_sta, "AutoTrigger, ON!");
      if (EEPROM.read(debug_mode_Address)) {
        client.publish("testing code", "Siren state set to on done!");
      }
    } else if (payloadStr.equals("Turn OFF")) {
      //node_mode = false;
      autoTrigger_state = false;
      client.publish(nodeStateSetManual_sta, "AutoTrigger, OFF!");
      if (EEPROM.read(debug_mode_Address)) {
        client.publish("testing code", "Siren state set to off done!");
      }
    }
  }

  if (topicStr.equals(nodeStateSetManual_sta)) {  //
    if (payloadStr.equals("????")) {
      if (autoTrigger_state == true) {
        client.publish(nodeStateSetManual_sta, "AutoTrigger, ON");
        if (EEPROM.read(debug_mode_Address)) {
          client.publish("testing code", "Siren state check done! AutoTrigger On");
        }
      } else if (autoTrigger_state == false) {
        client.publish(nodeStateSetManual_sta, "AutoTrigger, OFF");
        if (EEPROM.read(debug_mode_Address)) {
          client.publish("testing code", "Siren state check done! AutoTrigger Off");
        }
      }
    }
  }

  if (topicStr.equals(nodeOntimeConfig_listn)) {
    int commaIndex = payloadStr.indexOf(',');
    String valStr = payloadStr.substring(commaIndex + 2);

    int buffer = valStr.toInt();

    if (buffer > 9 && buffer < 3601) {
      Siren_on_time_in_sec = valStr.toInt();
      EEPROM.put(System_delay_Address, Siren_on_time_in_sec);
      EEPROM.commit();
      String tempPayLoad = "NewDelay, ";
      tempPayLoad += String(Siren_on_time_in_sec);
      client.publish(nodeOntimeConfig_sta, tempPayLoad.c_str());
      if (EEPROM.read(debug_mode_Address)) {
        client.publish("testing code", "delay Set successful!");
      }
    } else {
      client.publish(nodeOntimeConfig_sta, "Invalid:001");
      if (EEPROM.read(debug_mode_Address)) {
        client.publish("testing code", "delay Input data Invalid: min 10 (10s): max 3600 (1h)");
      }
    }
  }

  if (topicStr.equals(nodeOntimeConfig_sta)) {  // sta/floor3/LR/node/delay
    if (payloadStr.equals("????")) {
      String tempPayLoad = "Delay, ";
      tempPayLoad += String(Siren_on_time_in_sec);
      client.publish(nodeOntimeConfig_sta, tempPayLoad.c_str());
      if (EEPROM.read(debug_mode_Address)) {
        client.publish("testing code", "Delay check successful!");
      }
    }
  }

  if (topicStr.equals(nodeSystemState_listn)) {  // cmd/floor3/LR/node/system
    if (payloadStr.equals("Turn OFF")) {
      node_system_state = false;
      digitalWrite(siren, LOW);
      SirenON_OFF = false;
      Timer2.detach();
      EEPROM.write(System_state_Address, 0);
      EEPROM.commit();
      client.publish(nodeSystemState_send, "Sys, OFF!");
      if (EEPROM.read(debug_mode_Address)) {
        client.publish("testing code", "System turn off successful!");
      }
    }

    else if (payloadStr.equals("Turn ON")) {
      node_system_state = true;
      EEPROM.write(System_state_Address, 1);
      EEPROM.commit();
      client.publish(nodeSystemState_send, "Sys, ON!");
      if (EEPROM.read(debug_mode_Address)) {
        client.publish("testing code", "System turn on successful!");
      }
    }
    /*
    else if (payloadStr.equals("Turn ON") && node_system_state == true) {
      client.publish(nodeSystemState_send, "System is in On state already!");
    }
    */
  }

  if (topicStr.equals(nodeSystemState_send)) {  // sta/floor3/LR/node/system
    if (payloadStr.equals("????")) {
      if (node_system_state == false) {
        client.publish(nodeSystemState_send, "Sys, OFF");
        if (EEPROM.read(debug_mode_Address)) {
          client.publish("testing code", "System in turn off state");
        }
      } else if (node_system_state == true) {
        client.publish(nodeSystemState_send, "Sys, ON");
        if (EEPROM.read(debug_mode_Address)) {
          client.publish("testing code", "System in turn on state");
        }
      }
    }
  }

  if (topicStr.equals(resetReason_sta)) {
    if (payloadStr.equals("????")) {
      rebootStatus();
    }
  }
}
