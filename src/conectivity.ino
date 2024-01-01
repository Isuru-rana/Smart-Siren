int connection_timeout;
int temp_count;
int led_counter;

void WiFiConnect() {
  WiFi.begin(ssid, password);
  //node_mode = true;  // setting to independent mode

  connection_timeout = WiFi_timeout_counter;
  temp_count = 0;
  led_counter = WiFi_LED_timeout;

  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    if (led_counter > 0) {
      led_counter--;
      //ESP.wdtFeed();
    }
    if (led_counter == 0) {
      ledBlink(ST_CONNECT_WiFi);
      temp_count++;
      led_counter = WiFi_LED_timeout;
      //ESP.wdtFeed();
    }
    if (temp_count == connection_timeout) {
      node_mode = true;
      ESP.wdtFeed();
      temp_count = 0;
      break;
    }
    ESP.wdtFeed();
  }
  if (WiFi.status() != WL_CONNECTED) {
    ESP.wdtFeed();
    WiFiConnect();
  } 
}



void MQTTconnect() {

  connection_timeout = MQTT_timeout_counter;
  temp_count = 0;
  led_counter = MQTT_LED_timeout;

  if (WiFi.status() == WL_CONNECTED) {

    client.connect(device_ID, mqttUsername, mqttPassword);

    while (!client.connected()) {
      delay(100);
      if (led_counter > 0) {
        led_counter--;
      }
      if (led_counter == 0) {
        ledBlink(ST_CONNECT_MQTT);
        temp_count++;
        led_counter = MQTT_LED_timeout;
      }
      if (temp_count == connection_timeout) {
        node_mode = true;
        temp_count = 0;
        break;
      }
      ESP.wdtFeed();
    }
    if (!client.connected()) {
      ESP.wdtFeed();
      MQTTconnect();
    } 
    if (client.connected()) {
      node_mode = false;
      subscribeChannels();
      nodeStatusfunc();
      ESP.wdtFeed();
      delay(100);
      client.publish(startUp_message, "null"); // system started message
      client.publish(check_connection_send, "1");
    }
  } else {
    ESP.wdtFeed();
    WiFiConnect();
  }
}




void ledBlink(int time) {
  //digitalWrite(STLED, HIGH);
  //delay(time);
  digitalWrite(STLED, LOW);
  delay(time);
  digitalWrite(STLED, HIGH);
  //delay(time);
}
