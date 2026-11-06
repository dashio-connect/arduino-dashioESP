# DashIO

So, what is **DashIO**? It is a quick effortless way to connect your IoT device to your phone, tablet or iPad using the free **Dash IoT** app. It allows easy setup of controls such as Dials, Text Boxes, Charts, Graphs, Notifications..., from your IoT device. You can define the look and layout of the controls on your phone from your IoT device. There are three methods to connect to your phone; Bluetooth Low Energy (BLE), TCP or MQTT.

What is **dash** then? **dash** is an IoT platform based on an MQTT server with extra bits added in to allow you to store data, manage your devices, send notifications, share your devices, and save your **Dash** app setup.

## Discord Community

[![](https://img.shields.io/discord/1313341785430429747?color=5865F2&logo=Discord&style=flat-square)](https://discord.gg/fqvhNV3GQB)

Be a part of the DashIO community by joining our [Discord Server](https://discord.gg/fqvhNV3GQB)

## Documentation

For the **DashIO** ESP32 library documentation: <a href="https://dashio.io/guide-arduino-esp32">https://dashio.io/guide-arduino-esp32</a>

For the **DashIO** ESP8266 library documentation: <a href="https://dashio.io/guide-arduino-esp8266">https://dashio.io/guide-arduino-esp8266</a>

For the big picture on **DashIO**, take a look at our website: <a href="https://dashio.io">dashio.io</a>

For all documentation and software guides: <a href="https://dashio.io/documents">dashio.io/documents</a>


## Dash IoT Application

The **Dash** app is free and available for both Apple and Android devices. Use it to create beautiful and powerful user interfaces to you IoT devices.


<img src="https://dashio.io/wp-content/uploads/2020/11/IMG_4154.jpeg" width="200" />

<img src="https://dashio.io/wp-content/uploads/2020/12/IMG_4203.jpeg" width="600" />

## Release Notes

### 1.4.1 (2 November 2026)

- Fix MQTT messages being dropped when send batching is on (the 4th *DashioMQTT* constructor argument) on the ESP8266, and on the ESP32 with *esp32_mqtt_blocking = true*. A message that doesn't fit in the send buffer is no longer dropped: what is already waiting is published first. Layout pieces are now published straight away instead of being batched, so large layouts arrive complete.
- *DashioMQTT::end()* now publishes any batched messages before disconnecting, so they aren't lost.

### 1.4.0 (1 November 2026)

- ESP32 MQTT now runs in its own task so it no longer blocks *loop()*. Set *esp32_mqtt_blocking = true* for the previous behaviour.
- Added *DashioMQTT::incomingBufferSize* to set the incoming MQTT buffer size (default and minimum 512 bytes). Requires **Dashio** 1.2.5 or later.
- ESP32 WiFi power save is now off by default, which removes multi-second MQTT and TCP delays when BLE is also used. This uses more power. Set *wifi.powerSave = true* before *wifi.begin()* to restore power saving.
- MQTT keep alive increased to 60 s, adjustable with *keepAliveS* and *mqttTimeoutMs*.
- Slower BLE advertising interval (152.5 ms) to improve WiFi performance when BLE and WiFi are used together.
- Fix crash when a BLE client disconnects before its message is processed.
- Fix lost and corrupted BLE messages.
- Fix off-by-one in the mDNS service name buffer.
- Fix large layouts sometimes arriving incomplete over ESP32 MQTT. Layout pieces are now sent first, ahead of other messages, and messages wait for room (up to 3 s) instead of being dropped.
- Fix TCP blocking *loop()* when a phone leaves the WiFi without closing its connection. A client that stops taking data is now dropped when a write times out, so other phones can still connect.
- Examples use *setStatusCallback* instead of the deprecated *setOnConnectCallback*. The guides now explain how to set the clock from the Dash server.

### 1.3.2 (14 October 2026)

- Fix BLE init crash
- Allow MQTT QoS to be adjustable and Online/Offline messages to be optional

### 1.3.1 (8 October 2026)

- Repair ESP8266 compile linker

### 1.3.0 (26 November 2025)

- Added direct BLE stack NimBLE functionality. Only the original ESP32 uses NimBLE-Arduino. All others use the new functionality.

### 1.2.18 (21 July 2025)

- Improved BLE passkey pairing and bonding 

### 1.2.17 (9 July 2025)

- Fix BLE problems 

### 1.2.15 (8 April 2025)

- Use NimBLECharacteristicCallbacks onStatus callback to slow low down BLE messages during config to prevent buffer overflow.

### 1.2.14 (18 March 2025)

- Extend MQTT timeouts for slow network conditions

### 1.2.13 (8 March 2025)

- Added *connectTimeoutS* attribute to WiFi connection; the number of seconds of failed WiFi connection before the ESP reboots. Set to 0 to disable reboot.

### 1.2.12 (31 January 2025)

- Fix BLE restart advertising that is causing failure to reconnect.

### 1.2.11 (11 January 2025)

- Fix BLE incoming message errors associated with some Android phones/tablets.
- Fix BLE authentication completion.
- Make sure incoming BLE messages are correctly reported as BLE, not MQTT.

### 1.2.10 (5 January 2025)

- Fix "mutex" error now occurring for Arduino ESP32 boards V3.1.0

### 1.2.9 (30 December 2024)

- Fix BLE data corruption issues

### 1.2.8 (28 December 2024)

- Tidy up for NimBLE modifications

### 1.2.7 (28 December 2024)

- Fix for NimBLE task priority issue

### 1.2.6 (26 December 2024)

- Fix intermittent BLE message corruption

### 1.2.5 (18 December 2024)

- Fix BLE client allocation bug

### 1.2.4 (16 December 2024)

- Keeping pace with changes to NimBLE 2.1.0, with a fix to config (layout) downloading

### 1.2.3 (12 December 2024)

- Migrated to NimBLE 2.0.0

### 1.2.2 (11 December 2024)

- Fix change of use of compiler directive #endif by some Arduino IDE installations

### 1.2.1 (1 October 2024)

- Updated library properties

### 1.2.0 (30 September 2024)

- Release to Arduino library

