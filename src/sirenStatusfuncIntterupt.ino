void nodeStatusfunc() {
  if (flag1 == false) {  // this flag is used to stop executing "no response" in rutine at the code starts
    flag1 = true;
    if (EEPROM.read(debug_mode_Address)) {
      client.publish("testing code", "flag1 turn on");
    }
  }
  if (EEPROM.read(debug_mode_Address)) {
    client.publish("testing code", "Timer1 interrupt for check statues happend");
  }
  client.publish(check_connection_send, "1");

  // after sending status check signal, it will reset recive timer to check recive message in time
  Timer3.attach(timeoutT, notRespond);

  // subscribing to recive massage again for dobble check
  if (node_mode == false) {
    client.subscribe(check_connection_rec);
  }
  // reciveing and checking is on mqtt call back section
}
