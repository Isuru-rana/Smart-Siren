void callBack(char* topic, byte* payload, unsigned int length) {

  String payloadStr = "";
  String topicStr = topic;

  for (int i = 0; i < length; i++) {
    payloadStr += (char)payload[i];
  }

  //Serial.printf("Topic: %s , Payload: %s\n", topic, payloadStr);


  // if check connection message is recived from HA os,
  if (topicStr.equals(check_connection_rec)) {
    if (payloadStr.equals("online")) {
      Timer3.detach();

      if (node_state) {  // set node state to HA mode again if on Indipendent mode
        node_state = false;
        if (EEPROM.read(debug_mode_Address)) {
          client.publish("testing code", "response recived. turning off indipendent mode");
        }
      }
    }
  } else if (topicStr.equals(debug_mode_listn)) {
    if (payloadStr.equals("Turn ON")) {
      EEPROM.write(debug_mode_Address, 1);
      EEPROM.commit();
      client.publish(debug_mode_sta, "Debug mode is TURN ON");
    } else if (payloadStr.equals("Turn OFF")) {
      EEPROM.write(debug_mode_Address, 0);
      EEPROM.commit();
      client.publish(debug_mode_sta, "Debug mode is TURN OFF");
    }
  }

  if (topicStr.equals(SirenTopic_listn)) {
    if (payloadStr.equals("Turn ON")) {
      digitalWrite(siren, HIGH);
      SirenON_OFF = true;
      Timer2.attach(Siren_on_time_in_sec, sirenOfftime);

      if (EEPROM.read(debug_mode_Address)) {
        client.publish("testing code", "Siren Turn On with manual mqtt");
      }

    } else if (payloadStr.equals("Turn OFF")) {
      digitalWrite(siren, LOW);
      SirenON_OFF = false;
      Timer2.detach();
      if (EEPROM.read(debug_mode_Address)) {
        client.publish("testing code", "Siren Turn Off with manual mqtt");
      }
    }
  }

  else if (topicStr.equals(nodeStateSetManual_Listn)) {  //  set/floor3/LR/node/mode
    if (payloadStr.equals("Turn ON")) {
      node_state = true;
      client.publish(nodeStateSetManual_sta, "INDIPENDENT, ON");
    } else if (payloadStr.equals("Turn OFF")) {
      node_state = false;
      client.publish(nodeStateSetManual_sta, "INDIPENDENT, OFF");
    }
  }

  else if (topicStr.equals(nodeStateSetManual_sta)) {  //
    if (payloadStr.equals("????")) {
      if (node_state == true) {
        client.publish(nodeStateSetManual_sta, "INDIPENDENT, ON");
      } else if (node_state == false) {
        client.publish(nodeStateSetManual_sta, "INDIPENDENT, OFF");
      }
    }
  }

  else if (topicStr.equals(nodeOntimeConfig_listn)) {
    int commaIndex = payloadStr.indexOf(',');
    String valStr = payloadStr.substring(commaIndex + 2);

    int buffer = valStr.toInt();

    if (buffer > 10 && buffer < 3600) {
      Siren_on_time_in_sec = valStr.toInt();
      EEPROM.put(System_delay_Address, Siren_on_time_in_sec);
      EEPROM.commit();
      String tempPayLoad = "delay Set successful! new delay: ";
      tempPayLoad += String(Siren_on_time_in_sec);
      client.publish(nodeOntimeConfig_sta, tempPayLoad.c_str());
    } else {
      client.publish(nodeOntimeConfig_sta, "Invalid Input, Min: 10 (10s), max: 3600 (1h)");
    }
  }

  else if (topicStr.equals(nodeOntimeConfig_sta)) {  // sta/floor3/LR/node/delay
    if (payloadStr.equals("????")) {
      String tempPayLoad = "Delay, ";
      tempPayLoad += String(Siren_on_time_in_sec);
      client.publish(nodeOntimeConfig_sta, tempPayLoad.c_str());
    }
  }

  else if (topicStr.equals(nodeSystemState_listn)) {  // cmd/floor3/LR/node/system
    if (payloadStr.equals("Turn OFF")) {
      node_system_state = false;
      EEPROM.write(System_state_Address, 0);
      EEPROM.commit();
      client.publish(nodeSystemState_send, "Sys OFF!");
    }

    else if (payloadStr.equals("Turn ON") && node_system_state == false) {
      node_system_state = true;
      EEPROM.write(System_state_Address, 1);
      EEPROM.commit();
      client.publish(nodeSystemState_send, "Sys ON!");
    }
/*
    else if (payloadStr.equals("Turn ON") && node_system_state == true) {
      client.publish(nodeSystemState_send, "System is in On state already!");
    }
    */
  }

  else if (topicStr.equals(nodeSystemState_send)) {  // sta/floor3/LR/node/system
    if (payloadStr.equals("????")) {
      if (node_system_state == false) {
        client.publish(nodeSystemState_send, "System in TURN OFF state");
      } else if (node_system_state == true) {
        client.publish(nodeSystemState_send, "System in TURN ON state");
      }
    }
  }
}
