unsigned long previousMillisSiren = 0;       // Store the last time LED was updated
const long dotInterval = 200;                // Interval for dots (milliseconds)
const long dashInterval = 3 * dotInterval;   // Interval for dashes (milliseconds)
const long spaceInterval = 3 * dotInterval;  // Interval for spaces between characters (milliseconds)
int sosIndex = 0;                            // Index to iterate through SOS pattern

char sosPattern[] = ".-.-.-.--.--.--";  // SOS Morse code pattern (dots and dashes)


void siren_state_set(bool type, bool state) {
  if (type) {
    if (state) {
      digitalWrite(siren, LOW);
      lights_set(1);
    } else {
      digitalWrite(siren, HIGH);
      digitalWrite(light, HIGH);
    }
  } else {
    if (state) {
      unsigned long currentMillis = millis();  // Get the current time
      if (currentMillis - previousMillisSiren >= (sosPattern[sosIndex] == '.' ? dotInterval : dashInterval)) {
        // Save the last time the LED was updated
        previousMillisSiren = currentMillis;
        // Update the LED based on the current character in SOS pattern
        if (sosPattern[sosIndex] == '.') {
          digitalWrite(siren, HIGH);  // Dot (turn on LED)
          digitalWrite(light, HIGH);
        } else if (sosPattern[sosIndex] == '-') {
          digitalWrite(siren, LOW);  // Dash (turn off LED)
          digitalWrite(light, LOW);
        }
        // Move to the next character in the SOS pattern
        sosIndex = (sosIndex + 1) % sizeof(sosPattern);
        // If it's the end of an SOS character, add a space
        if (sosIndex == 0) {
          delay(spaceInterval);  // Add space between characters
        }
      }
    } else {
      digitalWrite(siren, HIGH);
      digitalWrite(light, HIGH);
    }
  }
}
unsigned long previousMillis = 0;
const long interval = 10;  // Interval at which to update LED brightness (milliseconds)
int brightness = 0;        // Current brightness of the LED
int fadeAmount = 5;        // How much to change the brightness each time
bool lightState = false;

void lights_set(bool state) {
  unsigned long currentMillis = millis();
  switch (light_pattern) {
    case 0:  //lights off
      digitalWrite(light, HIGH);
    case 1:  // lights fade
      if (currentMillis - previousMillis >= interval) {
        previousMillis = currentMillis;
        analogWrite(light, brightness);
        brightness = brightness + fadeAmount;
        if (brightness <= 0 || brightness >= 255) {
          fadeAmount = -fadeAmount;
        }
      }
    case 2:  // blink
      if (currentMillis - previousMillis >= interval) {
        previousMillis = currentMillis;
        if (lightState) {
          digitalWrite(lightState, LOW);
          lightState = false;
        } else {
          digitalWrite(lightState, HIGH);
          lightState = true;
        }
      }
  }
}