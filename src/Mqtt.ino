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
  
  else if (topicStr.equals(nodeStateSetManual_Listn)) {
    if (payloadStr.equals("on")) {
      node_state == true;
      client.publish(nodeStateSetSuccess_send, "INDIPENDENT, ON");
    }else if (payloadStr.equals("off")) {
      node_state == false;
      client.publish(nodeStateSetSuccess_send, "INDIPENDENT, OFF");
    }
  } 
  
  else if (topicStr.equals(nodeStateSetManual_sta)) {
    if (payloadStr.equals("???")) {
      if (node_state == true) {
        client.publish(nodeStateSetManual_sta, "INDIPENDENT, ON");
      } else if (node_state == false) {
        client.publish(nodeStateSetManual_sta, "INDIPENDENT, OFF");
      }
    }
  } 
  
  else if (topicStr.equals(nodeOntimeConfig_listn)) {
    int commaIndex = payloadStr.indexOf(',');
    String valStr = payloadStr.substring(commaIndex + 2);
    
    int buffer = valStr.toInt();

    if (buffer < 10 && buffer > 3600){
      Siren_on_time_in_sec = valStr.toInt();
      String tempPayLoad = "delay Set successful! new delay: ";
      tempPayLoad += String(Siren_on_time_in_sec);
    client.publish(nodeOntimeConfigSuccess_send, tempPayLoad.c_str());
    }
    else{
      client.publish(nodeOntimeConfigSuccess_send, "Invalid Input, Min: 10 (10s), max: 3600 (1h)");
    }    
  } 
  
  else if (topicStr.equals(nodeOntimeConfig_sta)) {
    if (payloadStr.equals("???")) {
      String tempPayLoad = "Delay, ";
      tempPayLoad += String(Siren_on_time_in_sec);
      client.publish(nodeOntimeConfig_sta, tempPayLoad.c_str());
    }
  }
}
