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

For previous releases, please refer to <a href="https://github.com/dashio-connect/arduino-dashio">github.com/dashio-connect/arduino-dashio</a>, which is now obsolete.

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

