/* ========= Siren + PIR + MQTT Node ESP01S Stable v.2.2.1 ========================
This code is developed for a node device in a home automation system that runs on Home Assistant OS.
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
*/


#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <RCSwitch.h>
#include <Ticker.h>
#include <EEPROM.h>

#define System_state_First_boot_Address 0
#define System_state_Address 1
#define debug_mode_Address 2
#define System_delay_Address 3

#define siren 1
const int PIR = 0;
#define STLED 2
#define RFIN 3

#define SirenTopic_send "sta/floor3/LR/node/siren"
#define SirenTopic_listn "cmd/floor3/LR/node/siren"

#define motionDetect_send "sta/floor3/LR/node/PIR"

#define RFRecivedData_send "data/floor3/LR/node/RF"

#define nodeStateSetManual_Listn "set/floor3/LR/node/mode"
//#define nodeStateSetSuccess_send "set_ok/floor3/LR/node/mode"
#define nodeStateSetManual_sta "sta/floor3/LR/node/mode"

#define nodeOntimeConfig_listn "config/floor3/LR/node/delay"
//#define nodeOntimeConfigSuccess_send "config_ok/floor3/LR/node/delay"
#define nodeOntimeConfig_sta "sta/floor3/LR/node/delay"

#define nodeSystemState_listn "cmd/floor3/LR/node/system"
//#define nodeSystemStateSuccess_send "cmd_ok/floor3/LR/node/system"
#define nodeSystemState_send "sta/floor3/LR/node/system"

#define check_connection_send "sta/floor3/LR/node/cnt"
#define check_connection_rec "sta/HA/sta"
#define check_connection_rec_payload "online"

#define debug_mode_listn "cmd/floor3/LR/node/debug"
#define debug_mode_sta "sta/floor3/LR/node/debug"

#define ST_CONNECT_WiFi 100
#define ST_CONNECT_MQTT 200

int Siren_on_time_in_sec = 600;  //10 mins = 600 [value is in secs.]

const char* ssid = "Home WiFi Network";
const char* password = "Whoareyou?";

const char* mqttServer = "192.168.8.119";
const int mqttPort = 1883;
const char* mqttUsername = "mqtt-home";
const char* mqttPassword = "Iamironman";

bool node_state = false;        // false == HA conected Mode || true == Indipendent mode
bool node_system_state = true;  // false == Auto indipendent Stop || true == Auto independant On

bool SirenON_OFF = false;  // Is siren on or off

bool flag1 = false;  //added this bool to stop execuding "noResponse()" function at code start without interrupt

const int timeoutT = 5;  //after sending status msg, timeout timer to recive HA state 5 == 5sec

//bool debug_mode = false;

WiFiClient espClient;
PubSubClient client(espClient);
RCSwitch rfReciver = RCSwitch();

Ticker Timer1;  // Timer for ststus checking
Ticker Timer2;  // Timer for Siren on time
Ticker Timer3;  // Timer to check status msg recived

void ICACHE_RAM_ATTR motionDetect();  // Used to pre determine the motion detect variable for hardwear interrupt

void setup() {

  EEPROM.begin(9);

  // get value for the state of the system from EEPROM and store it in system state bool
  if (EEPROM.read(System_state_First_boot_Address) == 1) {  // Check if this is the first boot
    node_system_state = EEPROM.read(System_state_Address);  // If this is first boot
    EEPROM.get(System_delay_Address,Siren_on_time_in_sec);
    //debug_mode = EEPROM.read(debug_mode_Address);
  } else {
    EEPROM.write(System_state_Address, 1);
    EEPROM.commit();
    EEPROM.write(System_state_First_boot_Address, 1);  //setting unit as not the first boot
    EEPROM.commit();
    EEPROM.write(debug_mode_Address, 0);
    EEPROM.commit();
    EEPROM.put(System_delay_Address, 600); // default delay set to 10 mins(600 sec)
    EEPROM.commit();
    Siren_on_time_in_sec = EEPROM.read(System_delay_Address);
    node_system_state = EEPROM.read(System_state_Address);
  }

  //Serial.begin(115200);
  pinMode(siren, OUTPUT);
  pinMode(STLED, OUTPUT);
  pinMode(PIR, INPUT);
  rfReciver.enableReceive(RFIN);

  attachInterrupt(PIR, motionDetect, FALLING);  // connect PIR as a hardwear interrupt

  Timer1.attach(30, nodeStatusfunc);

  ESP.wdtDisable();
  ESP.wdtEnable(WDTO_1S);  // Enabaling Watchdog timer for 1 sec.

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    ledBlink(ST_CONNECT_WiFi);
  }
  //Serial.println();
  //Serial.printf("WiFi Connected: %s\n", ssid);

  client.setServer(mqttServer, mqttPort);
  client.setCallback(callBack);

  client.connect("Floor3 Node", mqttUsername, mqttPassword);

  while (!client.connected()) {
    ledBlink(ST_CONNECT_MQTT);
  }
  //Serial.println("Connected to MQTT server");
  subscribeChannels();

  if (EEPROM.read(debug_mode_Address)) {
    client.publish("testing code", "passed Setup");
  }
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
