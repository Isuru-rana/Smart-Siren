void subscribeChannels() {
  
  client.subscribe(check_connection_rec);
  client.subscribe(SirenTopic_listn);
  client.subscribe(SirenTopic_send);

  client.subscribe(nodeStateSetManual_Listn);
  client.subscribe(nodeStateSetManual_sta);
  client.subscribe(nodeOntimeConfig_listn);
  client.subscribe(nodeOntimeConfig_sta);
  client.subscribe(nodeSystemState_listn);
  client.subscribe(nodeSystemState_send);
  client.subscribe(debug_mode_listn);
  client.subscribe(debug_mode_sta);
   client.subscribe(resetReason_sta);
  
  
 
  //client.subscribe();
  //client.subscribe();


}
