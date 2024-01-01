void reconnect() {
  int timeout = 30;
  bool status = false;

  if (WiFi.status() != WL_CONNECTED) {
    //Serial.print("WiFi disconnected");
    node_state = true;

    while (WiFi.status() != WL_CONNECTED && timeout > 0) {
      ledBlink(ST_CONNECT_WiFi);
      status = true;
      timeout--;
      ESP.wdtFeed();

      if (timeout < 1) {
        // Serial.println("  timeout! Retring....");
        timeout = 30;

        // Serial.print("WiFi disconnected");
      }
    }
  }

  if (status != false) {
    //Serial.println();
    //Serial.println("WiFi Connected");

    status = false;
    node_state = true;
  }

  if (WiFi.status() == WL_CONNECTED && !client.connected()) {
    //Serial.println("Mqtt Server disconnected");
    while (!client.connected() && WiFi.status() == WL_CONNECTED) {
      ledBlink(ST_CONNECT_MQTT);
      client.connect("Test Node 1", mqttUsername, mqttPassword);
      ESP.wdtFeed();
    }
    if (WiFi.status() != WL_CONNECTED){
      return;
    }

    //Serial.println("MQTT Server connected!");
    node_state = false;
    subscribeChannels();
    nodeStatusfunc();
    ESP.wdtFeed();
  }
}

void ledBlink(int time) {
  digitalWrite(STLED, LOW);
  delay(time);
  digitalWrite(STLED, HIGH);
  delay(time);
}