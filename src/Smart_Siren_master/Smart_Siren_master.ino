/* ========= Siren + PIR + MQTT node ESP01S stable v.2.3.0 ========================
This code is developed for a node device in a home automation system running on Home Assistant OS.
This node will manage an MQTT 3.1.1 client, a Siren device, and a PIR sensor.
The node has two different profiles called "Slave Mode" and "Independent Mode."

In "Slave Mode," the node works as an MQTT client, sending PIR sensor values to the MQTT broker.
The Siren remains in sleep mode until the broker sends a message to control the siren.

In "Independent Mode," the Siren works with the PIR, automatically turning on for 10 minutes before going to sleep.
Independent Mode will stay active until the MQTT broker connection is reestablished.

Updates
v2 - included RF receiver
v2.1.0 - 2023/10/02 - functions added: 
    Set the node to independent mode via MQTT
    Send independent mode status with MQTT request
    In this update:
        1. Users can manually set the node into independent mode, triggering
           the siren automatically with motion while sending an MQTT message to the MQTT broker.
           The activated siren can deactivate with MQTT.
        2. Users can ask for the independent mode status from the node, replying with the same topic
           to the broker.
    Bugs fixed: 
        Struck while connecting to the MQTT broker if WiFi disconnected bug fixed.
                      
v.2.1.1 - 2023/10/02 - modifications: 
    Fixed siren time changed for a variable that can be set by the user via MQTT.
    Bugs fixed: 
        Not responding to the config and state set setting fixed.
        MQTT stuck loop at WiFi disconnects enhanced.

v.2.1.2 - 2023/10/03 - bug fixed: 
    Delay input validation added.
    Invalid value input error message included.

v.2.2.0 - 2023/10/03 - feature added: 
    Manual Node off state
    In this feature, the user can manually turn off the siren to stop automatic
    independent mode change. This gives the user maintenance time for the MQTT broker
    while the device is in deactivated mode.

v.2.2.1 - 2023/10/06 - debug/feature added: 
    Added EEPROM to store configs and debug mode details.
    Added debug mode to limit unwanted MQTT messages.

v.2.2.2 - 2023/10/08 - bugs fixed: 
    Siren pin changed due to RX pin startup signals.
    Modified the code for MQTT reconnecting due to being struck at connecting.
    WiFi and MQTT reconnection functions divided into two separate functions.

v.2.2.3 - 2023/10/10 - improvements: 
    Siren manual system turn off can deactivate manual siren activation.
    Included new "Auto Trigger" and "Manual Trigger" modes for manual mod setup.
    Trigger Modes are added to EEPROM.
    Added Startup message.

v.2.2.4 - 2023/10/11 - feature added: 
    Siren On/off status values added.
    Debug mode flag added.

v.2.2.5 - 2023/10/11 - feature added: 
    Auto trigger siren sync messages added.
    MQTT reconnecting reboot added.
    Bugs Fixed: 
        Deactivating autotrigger mode issue fixed.

v.2.3.0 (Stable) - 2023/10/17 - fixes: 
    Booting restart fixed.
    Added reboot/monitoring.
    Code optimized.
    Connecting LED Indication changed for less power usage method.
    Added Independent mode if not connected to the network in the first 5 secs.
    Added device hostname setting.

v.3.0.0 (Stable)  - 2024/04/28 - code changes:
    Used WiFiManager+MQTT template (https://github.com/Isuru-rana/WiFiManager-MQTT.git)
    Removed RF Node functions
    Added Siren Light function with two styles
*/
#include <FS.h>
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>


#define TRIGGER_PIN 0

const char* mqtt_id = "IIZO Siren 1";
#define deviceName "IIZO Siren 1"
#define apPass "Isuru234"

#define siren 2
#define light 3
const int PIR = 1;

#define HA_Boot "homeassistant/status"
#define heartBeat "SirenNode/floor2/heartbeat"
#define lwt "{\"status\":\"Offline\"}"

#define cmd_siren "SirenNode/floor2/cmd/siren"
#define sta_siren "SirenNode/floor2/sta/siren"

#define cmd_siren_timer "SirenNode/floor2/cmd/siren/timer"
#define sta_siren_timer "SirenNode/floor2/sta/siren/timer"

#define cmd_lights "SirenNode/floor2/cmd/lights"
#define sta_lights "SirenNode/floor2/sta/lights"

#define cmd_lignts_pattern "SirenNode/floor2/cmd/lights/pattern"
#define sta_lignts_pattern "SirenNode/floor2/sta/lights/pattern"

#define sos_cmd "homeassistant/sos"
#define sos_sta "SirenNode/floor2/sta/SOS"

#define motionDetectsta "SirenNode/floor2/sta/motion"
#define sta_rssi "SirenNode/floor2/sta/rssi"


bool shouldSaveConfig = false;
#define mqtt_server "www.example.com"
#define mqtt_port "1883"
#define mqtt_user "mqtt Username"
#define mqtt_pass "mqtt Password"

int mqttPortInt = 1883;

WiFiClient espClient;
PubSubClient client(espClient);
WiFiManager wifiManager;

void saveConfigCallback() {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}

unsigned long previousMotionMillis = 0;
int motionTriggerTime = 5000;
bool motionFlag = false;
bool sosState = false;
bool SirenConState = false;
int sirenTimer = 600000;
void ICACHE_RAM_ATTR motionDetect();

void setup() {
  Serial.begin(115200);
  Serial.println();
  attachInterrupt(PIR, motionDetect, FALLING);  // connect PIR as a hardware interrupt
  pinMode(TRIGGER_PIN, INPUT);
  pinMode(siren, OUTPUT);
  digitalWrite(siren, HIGH);
  pinMode(PIR, INPUT);
  pinMode(light, OUTPUT);
  digitalWrite(light, HIGH);

  Serial.println("mounting FS...");
  if (SPIFFS.begin()) {
    Serial.println("mounted file system");
    if (SPIFFS.exists("/config.json")) {
      Serial.println("reading config file");
      File configFile = SPIFFS.open("/config.json", "r");
      if (configFile) {
        Serial.println("opened config file");
        size_t size = configFile.size();
        std::unique_ptr<char[]> buf(new char[size]);
        configFile.readBytes(buf.get(), size);
        DynamicJsonDocument jsonBuffer(2048);
        DeserializationError error = deserializeJson(jsonBuffer, buf.get());
        if (!error) {
          Serial.println("parsed json");
          strcpy(mqtt_server, jsonBuffer["mqtt_server"]);
          mqttPortInt = jsonBuffer["mqtt_port"];
          strcpy(mqtt_user, jsonBuffer["mqtt_user"]);
          strcpy(mqtt_pass, jsonBuffer["mqtt_pass"]);
          Serial.println("data copied");

        } else {
          Serial.println("failed to load json config");
        }
      }
    }
  } else {
    Serial.println("failed to mount FS");
  }

  WiFiManagerParameter custom_mqtt_server("server", "mqtt server", mqtt_server, 40);
  WiFiManagerParameter custom_mqtt_port("port", "mqtt port", mqtt_port, 6);
  WiFiManagerParameter custom_mqtt_user("user", "mqtt user", mqtt_user, 30);
  WiFiManagerParameter custom_mqtt_pass("pass", "mqtt pass", mqtt_pass, 20);


  wifiManager.setSaveConfigCallback(saveConfigCallback);
  wifiManager.addParameter(&custom_mqtt_server);
  wifiManager.addParameter(&custom_mqtt_port);
  wifiManager.addParameter(&custom_mqtt_user);
  wifiManager.addParameter(&custom_mqtt_pass);

  wifiManager.setHostname(deviceName);
  if (!wifiManager.autoConnect(deviceName, apPass)) {
    Serial.println("failed to connect and hit timeout");
    ESP.reset();
  }

  Serial.println("connected...yeey :)");
  strcpy(mqtt_server, custom_mqtt_server.getValue());
  strcpy(mqtt_port, custom_mqtt_port.getValue());
  strcpy(mqtt_user, custom_mqtt_user.getValue());
  strcpy(mqtt_pass, custom_mqtt_pass.getValue());

  if (shouldSaveConfig) {
    Serial.println("saving config");
    delay(100);
    mqttPortInt = atoi(mqtt_port);
    DynamicJsonDocument jsonBuffer(2048);
    JsonObject json = jsonBuffer.to<JsonObject>();
    json["mqtt_server"] = mqtt_server;
    json["mqtt_port"] = mqttPortInt;
    json["mqtt_user"] = mqtt_user;
    json["mqtt_pass"] = mqtt_pass;

    String jsonString;
    serializeJson(jsonBuffer, jsonString);

    Serial.println(jsonString);
    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile) {
      Serial.println("failed to open config file for writing");
    }
    configFile.println(jsonString);
    configFile.close();
  }
  SirenConState = true;

  Serial.println("local ip");
  Serial.println(WiFi.localIP());
}

void checkButton() {
  // check for button press
  if (digitalRead(TRIGGER_PIN) == LOW) {
    // poor mans debounce/press-hold, code not ideal for production
    delay(50);
    if (digitalRead(TRIGGER_PIN) == LOW) {
      Serial.println("Button Pressed");
      // still holding button for 3000 ms, reset settings, code not ideaa for production
      delay(3000);  // reset delay hold
      if (digitalRead(TRIGGER_PIN) == LOW) {
        Serial.println("Button Held");
        Serial.println("Erasing Config, restarting");
        wifiManager.resetSettings();
        if (SPIFFS.exists("/config.json")) {
          // Delete the /config.json file
          SPIFFS.remove("/config.json");
          Serial.println("/config.json file deleted.");
        } else {
          Serial.println("/config.json file does not exist.");
        }
        ESP.restart();
      }

      // start portal w delay
      Serial.println("Starting config portal");
      wifiManager.setConfigPortalTimeout(120);

      if (!wifiManager.startConfigPortal("OnDemandAP", "password")) {
        Serial.println("failed to connect or hit timeout");
        delay(3000);
        ESP.restart();
      } else {
        //if you get here you have connected to the WiFi
        Serial.println("connected...yeey :)");
        delay(1000);
      }
    }
  }
}

void MQTTsubscribe() {
  client.subscribe(HA_Boot);
  client.subscribe(cmd_siren);
  client.subscribe(cmd_lights);
  client.subscribe(cmd_lignts_pattern);
  client.subscribe(sos_cmd);
}

unsigned long lastReconnectAttempt = 0;

void reconnect() {
  const unsigned long reconnectInterval = 5000;  // 5 seconds
  SirenConState = false;

  // If connected, return immediately
  if (client.connected()) {
    return;
  }
  // Check for millis() rollover
  if (millis() < lastReconnectAttempt) {
    lastReconnectAttempt = millis();
  }
  // Try to reconnect at the specified interval
  if (millis() - lastReconnectAttempt > reconnectInterval) {
    lastReconnectAttempt = millis();
    checkButton();
    Serial.print("Attempting MQTT connection...");
    client.setServer(mqtt_server, mqttPortInt);
    client.setCallback(callBack);
    if (client.connect(mqtt_id, mqtt_user, mqtt_pass, heartBeat, 1, true, lwt)) {

      Serial.println("connected");
      SirenConState = true;
      MQTTsubscribe();
      mqttsend(1, 0, 0);
      mqttsend(0, 5, 0);
      mqttsend(0, 9, 0);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again later");
    }
  }

  // Perform software reset if millis() has rolled over
  if (millis() < lastReconnectAttempt) {
    Serial.println("millis() rollover detected, performing software reset");
    ESP.restart();
  }
}

bool checkBound(float newValue, float prevValue, float maxDiff) {
  return !isnan(newValue) && (newValue < prevValue - maxDiff || newValue > prevValue + maxDiff);
}

int32_t rssi;
int32_t rssiPrev = 0;

bool siren_state = false;
bool light_state = false;
int light_pattern = 1;

unsigned long sirenTimeFlag = 0;

void loop() {
  checkButton();
  rssi = WiFi.RSSI();

  if (WiFi.status() == WL_CONNECTED) {  //WiFi signal print display

    if (abs(rssi - rssiPrev) > 2) {
      mqttsend(0, 9, 0);
      rssiPrev = rssi;
    }
  }

  if (!client.connected()) {
    reconnect();
  }

  client.loop();

  if (siren_state) {
    sirenTimeFlag = millis();
    siren_state_set(1, true);
    if (light_state) {
      lights_set(light_pattern);
    }
  } 
  else if(!siren_state || (millis()- sirenTimeFlag) > sirenTimer) {
    siren_state_set(1, false);
    lights_set(0);
  }
  if (sosState) {
    siren_state_set(0, true);
  } else {
    siren_state_set(0, false);
  }

  if ((millis() - previousMotionMillis) > motionTriggerTime && motionFlag == true) {
    motionFlag = false;
    mqttsend(0, 3, 0);
  }
}
