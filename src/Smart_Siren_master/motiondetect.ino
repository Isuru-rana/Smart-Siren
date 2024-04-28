void motionDetect() {
  if (SirenConState) {
    previousMotionMillis = millis();
    motionFlag = true;
    mqttsend(0, 3, 1);
  } else {
    siren_state = true;
    siren_state_set(0, true);
  }
}