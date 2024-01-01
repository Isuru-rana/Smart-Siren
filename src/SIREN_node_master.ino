/* ========= Siren + PIR + MQTT node ESP01S stable v.2.1.0===================================
This code is developed for a node device in a home automation system that runs on Home Assistant OS.
This node will manage an MQTT 3.1.1 client, a Siren device, and a PIR sensor.
The node has two different profiles called "Slave Mode" and "Independent Mode."

In "Slave Mode," the node works as an MQTT client, sending PIR sensor values to the MQTT broker.
The Siren remains in sleep mode until the broker sends a message to control the siren.

In "Independent Mode," the Siren works with the PIR, automatically turning on for 10 minutes before going to sleep.
Independent Mode will stay active until the MQTT broker connection is reestablished.

Updates
v2 - included RF reciver
v2.1.0 - 2023/10/02 - function added: Set the node to independent mode via MQTT
                                      Send independent mode status with MQTT request
                      In this update,
                      1. Users can manually set the node into independent mode, triggering
                      the siren automatically with motion while sending an MQTT message to the MQTT broker.
                      Activated siren can deactivate with MQTT.
                      2. User can ask for the indipendent mode status from the node, replying with the same topic
                      to the broker.

                      bugs fixed: Struck while connecting to MQTT broker if WiFi disconnected bug fixed.

*/

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <RCSwitch.h>
#include <Ticker.h>

#define siren 1
const int PIR = 0;
#define STLED 2
#define RFIN 3

#define SirenTopic_send "sta/floor3/LR/node/siren"
#define SirenTopic_listn "cmd/floor3/LR/node/siren"
#define motionDetect_send "sta/floor3/LR/node/PIR"
#define RFRecivedData_send "sta/floor3/LR/node/RF"
#define nodeStateSetManual_Listn "cmd/floor3/LR/node/mode"
#define nodeStateSetManual_sta "sta/floor3/LR/node/mode"

#define check_connection_send "sta/floor3/LR/node/cnt"

#define check_connection_rec "sta/HA/sta"
#define check_connection_rec_payload "online"

#define ST_CONNECT_WiFi 100
#define ST_CONNECT_MQTT 200
#define Siren_on_time_in_sec 60  //10 mins = 600

const char* ssid = "Home WiFi Network";
const char* password = "Whoareyou?";

const char* mqttServer = "192.168.8.119";
const int mqttPort = 1883;
const char* mqttUsername = "mqtt-home";
const char* mqttPassword = "Iamironman";

bool node_state = false;  // false == HA conected Mode || true == Indipendent mode

bool SirenON_OFF = false;  // Is siren on or off

bool flag1 = false;  //added this bool to stop execuding "noResponse()" function at code start without interrupt

const int timeoutT = 5;  //after sending status msg, timeout timer to recive HA state 5 == 5sec

WiFiClient espClient;
PubSubClient client(espClient);
RCSwitch rfReciver = RCSwitch();

Ticker Timer1;  // Timer for ststus checking
Ticker Timer2;  // Timer for Siren on time
Ticker Timer3;  // Timer to check status msg recived

void ICACHE_RAM_ATTR motionDetect();  // Used to pre determine the motion detect variable for hardwear interrupt

void setup() {

  //Serial.begin(115200);
  pinMode(siren, OUTPUT);
  pinMode(STLED, OUTPUT);
  pinMode(PIR, INPUT);
  rfReciver.enableReceive(RFIN); 

  attachInterrupt(PIR, motionDetect, FALLING);  // connect PIR as a hardwear interrupt

  Timer1.attach(30, nodeStatusfunc);

  ESP.wdtDisable();
  ESP.wdtEnable(WDTO_1S); // Enabaling Watchdog timer for 1 sec.

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    ledBlink(ST_CONNECT_WiFi);
  }
  //Serial.println();
  //Serial.printf("WiFi Connected: %s\n", ssid);

  client.setServer(mqttServer, mqttPort);
  client.setCallback(callBack);

  client.connect("Siren Node 1", mqttUsername, mqttPassword);

  while (!client.connected()) {
    ledBlink(ST_CONNECT_MQTT);
  }
  //Serial.println("Connected to MQTT server");

  client.subscribe(check_connection_rec);
  client.subscribe(SirenTopic_listn);

  client.publish("testing code", "passed Setup");
}

bool temp1 = false;
void loop() {
  if (!temp1) {
    client.publish("testing code", "Loop started");
    temp1 = true;
  }

  if (WiFi.status() != WL_CONNECTED || !client.connected()) {
    reconnect();
  }

  client.loop();
  rutine();

  if (rfReciver.available()){
    rfRecived(rfReciver.getReceivedValue(),rfReciver.getReceivedBitlength(),rfReciver.getReceivedDelay(),rfReciver.getReceivedProtocol());
     rfReciver.resetAvailable();
  }

  ESP.wdtFeed();
}
