void callBack(char* topic, byte* payload, unsigned int length) {

  String payloadStr = "";
  String topicStr = topic;

  for (int i = 0; i < length; i++) {
    payloadStr += (char)payload[i];
  }

  //Serial.printf("Topic: %s , Payload: %s\n", topic, payloadStr);


  // if check connection message is recived from HA os,
  if (topicStr.equals(check_connection_rec)) {
    if (payloadStr.equals("online")) {
      Timer3.detach();

      if (node_state) {  // set node state to HA mode again if on Indipendent mode
        node_state = false;
        client.publish("testing code", "response recived. turning off indipendent mode");
      }
    }
  }

  if (topicStr.equals(SirenTopic_listn)) {
    if (payloadStr.equals("On")) {
      digitalWrite(siren, HIGH);
      SirenON_OFF = true;
      Timer2.attach(Siren_on_time_in_sec, sirenOfftime);


      client.publish("testing code", "Siren Turn On with manual mqtt");

    } else if (payloadStr.equals("Off")) {
      digitalWrite(siren, LOW);
      SirenON_OFF = false;
      Timer2.detach();
      client.publish("testing code", "Siren Turn Off with manual mqtt");
    }
  }
}
