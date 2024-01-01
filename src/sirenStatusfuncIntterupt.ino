void nodeStatusfunc() {

  if (EEPROM.read(debug_mode_Address)) {
    client.publish("testing code", "Timer1 interrupt for check statues happend");
  }
  client.publish(check_connection_send, "1");
  Timer3.attach(timeoutT, notRespond);  // after sending status check signal, it will reset recive timer to check recive message in time
}

void notRespond() {
  // set siren into indipendent mode and reset timer settied from timer1 interrupt
  if (node_system_state == true) {
    node_mode = true;
    //client.subscribe(check_connection_rec);

    if (EEPROM.read(debug_mode_Address)) {
      client.publish("testing code", "Timer3 respose time exeded. indipendent mode ON");
    }

    Timer3.detach();

    if (client.connected()) {
      subscribeChannels();
    }
  } else if (node_system_state == false) {
    Timer3.detach();
    if (EEPROM.read(debug_mode_Address)) {
      client.publish("testing code", "system is in deactive mode: timer resets");
    }
  }
  ESP.wdtFeed();
}