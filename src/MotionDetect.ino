void motionDetect() {

  if (EEPROM.read(debug_mode_Address)) {
    client.publish("testing code", "Hardware Interrupt happens");
  }

  if (node_mode == false) {
    //client.publish("testing code", "Hardware Interrupt goes to HA mode signal send");
    client.publish(motionDetect_send, "True");
  }

  if (node_system_state == true) {
    if (node_mode == true || autoTrigger_state == true) {

      if (EEPROM.read(debug_mode_Address)) {
        client.publish("testing code", "Hardware Interrupt goes to Indipendent mode siren on");
      }

      digitalWrite(siren, HIGH);
      SirenON_OFF = true;
      client.publish(SirenTopic_send, "Siren, ON");
      Timer2.attach(Siren_on_time_in_sec, sirenOfftime);

      if (EEPROM.read(debug_mode_Address)) {
        client.publish("testing code", "Timer for Indipendent mode siren on is started");
      }
    }
  } else if (node_system_state == false) {
    if (EEPROM.read(debug_mode_Address)) {
      client.publish("testing code", "System is in deactive mode");
    }
  }
}
