int timeout = 30;
//bool status = false;

void reconnect() {

  WiFiReconnect();

  if (WiFi.status() == WL_CONNECTED) {
    MQTTReconnect();
  }
  else {
    WiFiReconnect();
  }

}



void WiFiReconnect() {
  if (WiFi.status() != WL_CONNECTED) {
    node_state = true; // setting to independent mode

    while (WiFi.status() != WL_CONNECTED && timeout > 0) {
      ledBlink(ST_CONNECT_WiFi);
      timeout--;
      ESP.wdtFeed();

      if (timeout < 1) {
        timeout = 30;
      }
    }
  }
}

void MQTTReconnect() {
  if (WiFi.status() == WL_CONNECTED && !client.connected()) {
    while (!client.connected()) {
      ledBlink(ST_CONNECT_MQTT);
      client.connect("Test Node 1", mqttUsername, mqttPassword);
      ESP.wdtFeed();
      if (WiFi.status() != WL_CONNECTED) {
        WiFiReconnect();
      }
    }
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
