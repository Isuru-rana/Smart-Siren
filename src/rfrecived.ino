void rfRecived(unsigned long data, unsigned int length, unsigned int pulse_length, unsigned int protocol) {

  const char* b = dec2binWzerofill(data, length);

  char address[9];
  char state[9];

  strncpy(address, b, 8);
  address[8] = '\0';
  strncpy(state, b + 8, 8);
  state[8] = '\0';

  int num1 = strtol(address, NULL, 2);

bool isTrue = strcmp(state, "01010101") == 0;

String mqttMessage = "Address, " + String(num1) + ", state, " + (isTrue ? "true" : "false");

   client.publish(RFRecivedData_send, mqttMessage.c_str(), true);
}


static char* dec2binWzerofill(unsigned long Dec, unsigned int bitLength) {
  static char bin[64];
  unsigned int i = 0;

  while (Dec > 0) {
    bin[32 + i++] = ((Dec & 1) > 0) ? '1' : '0';
    Dec = Dec >> 1;
  }

  for (unsigned int j = 0; j < bitLength; j++) {
    if (j >= bitLength - i) {
      bin[j] = bin[31 + i - (j - (bitLength - i))];
    } else {
      bin[j] = '0';
    }
  }
  bin[bitLength] = '\0';

  return bin;
}