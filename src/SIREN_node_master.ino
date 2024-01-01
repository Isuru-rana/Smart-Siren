/* ========= Siren + PIR + MQTT node ESP01S stable v.2.2.7 ========================
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

v.2.2.6 - Description Missing

v.2.2.7 - 2023/10/17 - Description Missing

*/


#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <RCSwitch.h>
#include <Ticker.h>
#include <EEPROM.h>

#define System_state_First_boot_Address 0
#define System_state_Address 1
#define System_autoTrigger_Address 2
#define debug_mode_Address 3
#define System_delay_Address 4

#define siren 0
const int PIR = 1;
#define STLED 2
#define RFIN 3
/*
 *  cmd - commands
 *  data - data transfer
 *  set - configurable inputs
 *  sta - status indicators 
 */
#define SirenTopic_listn "cmd/floor3/LR/node/siren"
#define SirenTopic_send "sta/floor3/LR/node/siren"

#define nodeSystemState_listn "cmd/floor3/LR/node/system"
#define nodeSystemState_send "sta/floor3/LR/node/system"

#define debug_mode_listn "cmd/floor3/LR/node/debug"
#define debug_mode_sta "sta/floor3/LR/node/debug"

#define nodeStateSetManual_Listn "cmd/floor3/LR/node/mode"
#define nodeStateSetManual_sta "sta/floor3/LR/node/mode"

//configs
#define nodeOntimeConfig_listn "set/floor3/LR/node/delay"
#define nodeOntimeConfig_sta "sta/floor3/LR/node/delay"

#define check_connection_send "sta/floor3/LR/node/cnt"
#define check_connection_rec "sta/HA/sta"
#define check_connection_rec_payload "online"

#define motionDetect_send "sta/floor3/LR/node/PIR"
#define startUp_message "sta/floor3/LR/node/start"

//RF data transmit
#define RFRecivedData_send "data/floor3/LR/node/RF"

#define ST_CONNECT_WiFi 100
#define ST_CONNECT_MQTT 200
#define WiFi_LED_pattern_timeout 30

int Siren_on_time_in_sec = 600;  //10 mins = 600 [value is in secs.]

const char* ssid = "Home WiFi Network";
const char* password = "Whoareyou?";

const char* mqttServer = "192.168.8.119";
const int mqttPort = 1883;
const char* mqttUsername = "mqtt-home";
const char* mqttPassword = "Iamironman";

bool node_mode = false;  // false == HA conected Mode || true == Indipendent mode
bool autoTrigger_state;  // false == Auto trigger Off manually ||  true == Auto Trigger On
bool node_system_state;  // false == System turn off || true == System turn ON
bool debug_mode_state = false;
bool startUp_flag = true;

bool SirenON_OFF = false;  // Is siren on or off

bool flag1 = false;  //added this bool to stop execuding "noResponse()" function at code start without interrupt

const int timeoutT = 5;  //after sending status msg, timeout timer to recive HA state 5 == 5sec



WiFiClient espClient;
PubSubClient client(espClient);
RCSwitch rfReciver = RCSwitch();

Ticker Timer1;  // Timer for status checking
Ticker Timer2;  // Timer for Siren on time
Ticker Timer3;  // Timer to check status msg recived

void ICACHE_RAM_ATTR motionDetect();  // Used to pre determine the motion detect variable for hardwear interrupt

void setup() {

  EEPROM.begin(9);

  // get value for the state of the system from EEPROM and store it in system state bool
  if (EEPROM.read(System_state_First_boot_Address) == 1) {  // Check if this is the first boot
    // If this is not the first boot
    node_system_state = EEPROM.read(System_state_Address);        // get previous system state (On or off)
    autoTrigger_state = EEPROM.read(System_autoTrigger_Address);  // get previous Auto trigger mode
    EEPROM.get(System_delay_Address, Siren_on_time_in_sec);       // get last delay input
  }

  else {
    //if this is the first boot
    EEPROM.write(System_state_First_boot_Address, 1);  //set node as not the first boot
    EEPROM.commit();
    EEPROM.write(System_state_Address, 1);  // set system as turn on mode
    EEPROM.commit();
    EEPROM.write(System_autoTrigger_Address, 0);  // set autotrigger mode as off
    EEPROM.commit();
    EEPROM.put(System_delay_Address, 600);  // set default delay to 10 mins(600 sec)
    EEPROM.commit();
    EEPROM.write(debug_mode_Address, 0);  // set debug mode as off
    EEPROM.commit();

    // sync written values to RAM variables
    Siren_on_time_in_sec = EEPROM.read(System_delay_Address);
    node_system_state = EEPROM.read(System_state_Address);
    autoTrigger_state = EEPROM.read(System_autoTrigger_Address);
  }

  //Serial.begin(115200);
  pinMode(siren, OUTPUT);
  pinMode(STLED, OUTPUT);
  pinMode(PIR, INPUT);

  rfReciver.enableReceive(RFIN);

  Timer1.attach(30, nodeStatusfunc);

  ESP.wdtDisable();
  ESP.wdtEnable(WDTO_1S);  // Enabaling Watchdog timer for 1 sec.

  attachInterrupt(PIR, motionDetect, FALLING);  // connect PIR as a hardware interrupt

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    WiFi_LED_pattern_timeout--;
    if (WiFi_LED_pattern_timeout < 0) {
      ledBlink(ST_CONNECT_WiFi);
    }
    ESP.wdtFeed();
  }

  client.setServer(mqttServer, mqttPort);
  client.setCallback(callBack);

  client.connect("Floor3 Node", mqttUsername, mqttPassword);
  /*
  while (!client.connected()) {
    ledBlink(ST_CONNECT_MQTT);
  }
  */
  MQTTConnect();
  //Serial.println("Connected to MQTT server");
  subscribeChannels();

  if (EEPROM.read(debug_mode_Address)) {
    client.publish("testing code", "passed Setup");
  }

  client.publish(startUp_message, "");
  nodeStatusfunc();
}

bool temp1 = false;

void loop() {
  if (!temp1) {
    if (EEPROM.read(debug_mode_Address)) {
      client.publish("testing code", "Loop started");
      temp1 = true;
    }
  }

  if (WiFi.status() != WL_CONNECTED || !client.connected()) {
    reconnect();
  }

  client.loop();
  rutine();

  if (rfReciver.available()) {
    rfRecived(rfReciver.getReceivedValue(), rfReciver.getReceivedBitlength(), rfReciver.getReceivedDelay(), rfReciver.getReceivedProtocol());
    rfReciver.resetAvailable();
  }

  ESP.wdtFeed();
}
