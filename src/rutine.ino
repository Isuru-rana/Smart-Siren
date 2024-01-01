
void rutine() {
}

void sirenOfftime() {
  if (EEPROM.read(debug_mode_Address)) {
    client.publish("testing code", "Siren On time exeeded.came to siren off function");
  }

  digitalWrite(siren, LOW);
  SirenON_OFF = false;

  Timer2.detach();

  if (node_state == false) {
    client.publish(SirenTopic_send, "0");
  }
}

void notRespond() {
  // set siren into indipendent mode and reset timer settied from timer1 interrupt
  if (node_system_state == true) {
    node_state = true;
    //statusTimer = 0;
    //statusTimer = millis();
    client.subscribe(check_connection_rec);
    if (EEPROM.read(debug_mode_Address)) {
      client.publish("testing code", "Timer3 respose time exeded. indipendent mode ON");
    }
    Timer3.detach();
  } else if (node_system_state == false) {
    Timer3.detach();
    if (EEPROM.read(debug_mode_Address)) {
      client.publish("testing code", "system is in deactive mode: timer resets");
    }
  }
}
