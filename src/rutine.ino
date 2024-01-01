
void rutine() {
}

void sirenOfftime() {

  client.publish("testing code", "Siren On time exeeded.came to siren off function");

  digitalWrite(siren, LOW);
  SirenON_OFF = false;

  Timer2.detach();

  if (node_state == false) {
    client.publish(SirenTopic_send, "0");
  }
}

void notRespond() {
  // set siren into indipendent mode and reset timer settied from timer1 interrupt
  node_state = true;
  //statusTimer = 0;
  //statusTimer = millis();
  client.subscribe(check_connection_rec);

  client.publish("testing code", "Timer3 respose time exeded. indipendent mode ON");

  Timer3.detach();
}