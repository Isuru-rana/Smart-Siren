void motionDetect() {


  client.publish("testing code", "Hardware Interrupt happens");


  if (node_state == false) {
    //client.publish("testing code", "Hardware Interrupt goes to HA mode signal send");
    client.publish(motionDetect_send, "true");
  }
  if (node_system_state == true) {
    if (node_state == true) {
      client.publish("testing code", "Hardware Interrupt goes to Indipendent mode siren on");
      digitalWrite(siren, HIGH);
      SirenON_OFF = true;
      Timer2.attach(Siren_on_time_in_sec, sirenOfftime);
      client.publish("testing code", "Timer for Indipendent mode siren on is started");
    }
  } else if (node_system_state == false){
    client.publish("testing code", "System is in deactive mode");
    return;
  }
}
