# Smart-Siren
Welcome to the IoTSirenMotion repository! 🚨🌐

# Overview:
This device, powered by the ESP01s (ESP8266) module, is a versatile IoT-enabled Siren with an integrated Motion sensor. Designed for seamless integration into smart home setups, it connects effortlessly to your WiFi network and communicates with an MQTT broker to provide comprehensive security features.

# Key Features:

## Dual Modes:

Independent Mode: The device operates independently, triggering the siren automatically upon motion detection using the integrated sensor.
Slave Mode: The device acts as a slave, sending motion sensor readings to the MQTT broker while the siren auto-trigger feature is disabled.
## MQTT Connectivity:

Connects to an MQTT broker over WiFi, ensuring reliable communication for both sending and receiving data.

## Manual and Auto-Trigger Control:

The siren can be manually activated or deactivated using MQTT messages.
Users can set up the Auto-Trigger mode by sending specific MQTT messages, allowing for customized automation.

### How It Works:
Upon power-up, the device intelligently selects between Independent and Slave Modes based on its connectivity to the MQTT broker. In Independent Mode, the siren auto-triggers upon motion detection, providing an immediate alert. In Slave Mode, the device focuses on sending accurate motion sensor readings to the MQTT broker, offering a non-intrusive monitoring option.

# Getting Started:

Flash the ESP01s with the provided firmware.
Connect the device to your WiFi network.
Configure MQTT settings.
Power up the device and experience the flexibility of the Independent and Slave Modes.

Explore the provided code, documentation, and examples to make the most of this IoT Siren device. Enhance your security setup today with IoTSirenMotion! 🛡️🔔

