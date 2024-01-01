
void rutine() {
}

void sirenOfftime() {
  if (EEPROM.read(debug_mode_Address)) {
    client.publish("testing code", "Siren On time exeeded.came to siren off function");
  }

  digitalWrite(siren, LOW);
  SirenON_OFF = false;
  client.publish(SirenTopic_send, "Siren, OFF");

  Timer2.detach();
}

void rebootStatus(){
  String tempPayLoad = "reset Reason, ";
  tempPayLoad += String(ESP.getResetReason());
  client.publish(resetReason_sta, tempPayLoad.c_str());
}
