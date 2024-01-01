void subscribeChannels() {
  client.subscribe(check_connection_rec);
  client.subscribe(SirenTopic_listn);


  client.subscribe(nodeStateSetManual_Listn);
  client.subscribe(nodeStateSetManual_sta);
  client.subscribe(nodeOntimeConfig_listn);
  client.subscribe(nodeOntimeConfig_sta);
  //client.subscribe();

}
