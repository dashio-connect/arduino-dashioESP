<h1 id="toc_0">Dash IoT Guide (ESP8266)</h1>

**12 February 2025**

This guide demonstrates how to make TCP and MQTT connections to an ESP8266 IoT device using the Arduino or PlatformIO IDE and the **Dash** Adruino library. It also shows how to load user controls (widgets) into your mobile device to monitor and control an IoT device.

<p>The MQTT connection discussed in this guide is SSL/TLS enabled and we highly recommend that you only use SSL/TLS connections. The <strong>Dash</strong> MQTT server only accepts SSL connections. However, the ESP8266 does not include hardware for creating trust with certificates. Therefore, the MQTT connection is encrypted, but trust is not established.</p>

<h2 id="toc_1">Getting Started</h2>

<p>For the big picture on DashIO, take a look at our website: <a href="https://dashio.io">dashio.io</a></p>

<p>For the Dash arduino library: <a href="https://github.com/dashio-connect/arduino-dashioESP">github.com/dashio-connect/arduino-dashioESP</a></p>

<h2 id="toc_2">Requirements</h2>

<p>Grab an ESP8266 board, Arduino IDE and follow this guide. This example should work on ESP8266 development boards available for the Arduino IDE.</p>

You will need to install the <strong>Dash</strong> app on your mobile phone or tablet. 

If you'd like to connect to your IoT devices through the **Dash** MQTT broker, setup an account on the <a href="https://dashio.io">dashio.io</a> website.

<h2 id="toc_3">Install</h2>

<h3 id="toc_4">Arduino IDE</h3>

If you haven't yet installed the ESP8266 Arduino IDE support from espressif, please check the following link for instructions: <a href="https://github.com/esp8266/Arduino#installing-with-boards-manager">https://github.com/esp8266/Arduino#installing-with-boards-manager</a>

You will need to add the **DashESP** library into your project.  It is included in the Arduino IDE Library manager. Search the library manager for the library titled "DashioESP" and install. The Arduino IDE may also ask you to install the following libraries (dependencies). Please make sure they are all installed from the Arduino IDE Library Manager.

- **Dashio** is the core messageing library used to manage messages.
- **MQTT** by Joël Gähwiler is required for MQTT messaging.
- **Preferences** by Volodymyr Shymanskyy is used storing WiFi and login credentials etc.
- **NimBLE-Arduino** by h2zero is required for BLE messaging for the ESP32. It is not required for the ESP8266.

The **arduino-timer** library is also used for the WiFi and MQTT connection and is available in the Arduino IDE Library Manager. Search the library manager for the library titled "arduino-timer" by Michael Contreras and install.

<h3 id="toc_5">PlatformIO</h3>

Using **Dash** with the **PlatformIO** IDE is easy.

1. Install the **Dash** library into your project. The easiest way to do this is to download the <a href="https://github.com/dashio-connect/arduino-dashio">**arduino-dashio**</a> library  into the ***lib*** directory of your project.
2. Install the libraries listed above using the PlatformIO **Libraries** manager.
3. Edit the ***platformio.ini*** file for you project by adding ```lib_ldf_mode = deep``` to enable all the required files to be found and also add ```monitor_speed = 115200``` to set the required serial monitor speed.

Your finished ***platformio.ini*** file should look something like this:

```
[env:huzzah]
platform = espressif8266
board = huzzah
framework = arduino
lib_deps = 
	contrem/arduino-timer@^2.3.1
	256dpi/MQTT@^2.5.0
	vshymanskyy/Preferences@^2.0.0
lib_ldf_mode = deep
monitor_speed = 115200
```

<h2 id="toc_6">Guide</h2>

<h3 id="toc_7">TCP Basics</h3>

<p>Lets have a look at a TCP connection.</p>

```
#include "DashioESP.h"

char WIFI_SSID[] = "yourWiFiSSID";
char WIFI_PASSWORD[] = "yourWiFiPassword";

DashDevice dashDevice("ESP8266_Type");
DashTCP    tcp_con(&dashDevice, true);
DashWiFi   wifi;

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    dashDevice.setup(wifi.macAddress(), "Jim Name"); // unique deviceID, and device name
    wifi.attachConnection(&tcp_con);
    wifi.begin(WIFI_SSID, WIFI_PASSWORD);
}

void loop() {
    wifi.run();
}
```
After the <code>#include "DashioESP.h"</code> we create a device with the <em>device_type</em> as its only attribute. We also create a TCP connection with ```DashTCP    tcp_con(&dashDevice, true);```, with the newly created device being an attribute to the connection. The second attribute of the TCP connection enables the received and transmitted messages to be printed to the serial monitor to make debugging easier.

By default, TCP port 5650 is used and only one TCP client can connect at a time. These values can be changed when initialising the TCP connection as follows:

```
DashTCP    tcp_con(&dashDevice, true, 5650, 5);
```

The third attribute specifies the port (5650 in this example) and the fourth is the number of TCP clients allowed (5 in this case). Increasing the number of TCP clients affects memory usage and the maximum value will depend on the capabilities of the processor you are using and how much memory your code uses.

In the <code>setup()</code> function we call the device setup with <code>dashDevice.setup(wifi.macAddress(), "Jim Name");</code>. The two parameters describe the device to the <strong>Dash</strong> app to allow it uniquely identify each device. The wifi object supplies the <em>mac</em> address to the device, to be used as a unique <em>device_ID</em>.

<p>We then attach the TCP connection to the <code>wifi</code> with <code>wifi.attachConnection(&tcp_con);</code>. This enables the <code>wifi</code> object to manage the TCP connection from here on in.</p>

<p>We start the WiFi <code>wifi.begin(WIFI_SSID, WIFI_PASSWORD);</code> with the WiFi SSID and password. This starts the wifi and TCP connection and sets up the the mDNS (also called Bonjour or Zeroconf) to make the device discoverable on the <strong>Dash</strong> app.</p>

<p><code>wifi.run();</code> is used to regularly check that the WiFi is connected and manage the attached connection. </p>

<p>This device is discoverable by the <strong>Dash</strong> app. You can also discover your IoT device using a third party Bonjour/Zeroconf discovery tool. The mDNS service will be "_DashIO._tcp." and individual IoT devices on this service are identified by their mac address (<em>device_ID</em>).</p>

<p>In the code above, make sure you replace <strong>yourWiFiSSID</strong> with your WiFi SSID and <strong>yourWiFiPassword</strong> with your WiFI password. Setup the Arduino IDE serial monitor with 115200 baud and run the code. You will see messages on the Arduino serial monitor, and after a few attempts, your IoT device should connect to your WiFi and start the mDNS service. Then run the <strong>Dash</strong> app on your mobile device and you will see connection "WHO" messages on the Arduino serial monitor.</p>

<p>Lets add Dial control messages that are sent to the <strong>Dash</strong> app every second. We use a timer and send a Dial value message from the loop every second.</p>

```
#include "DashioESP.h"
#include <arduino-timer.h>

char WIFI_SSID[] = "yourWiFiSSID";
char WIFI_PASSWORD[] = "yourWiFiPassword";

DashDevice dashDevice("ESP8266_Type");
DashTCP    tcp_con(&dashDevice, true);
DashWiFi   wifi;

auto timer = timer_create_default();
bool oneSecond = false; // Set by timer every second.

static bool onTimerCallback(void *argument) {
  oneSecond = true;
  return true; // to repeat the timer action - false to stop
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    dashDevice.setup(wifi.macAddress(), "Jim Name"); // unique deviceID, and device name
    wifi.attachConnection(&tcp_con);
    wifi.begin(WIFI_SSID, WIFI_PASSWORD);

    timer.every(1000, onTimerCallback); // 1000ms
}

void loop() {
    timer.tick();
    wifi.run();

    if (oneSecond) { // Tasks to occur every second
        oneSecond = false;
        tcp_con.sendMessage(dashDevice.getDialMessage("D01", int(random(0, 100))));
    }
}
```

<p>The line <code>tcp_con.sendMessage(dashDevice.getDialMessage("D01", int(random(0, 100))));</code> creates the message with two parameters. The first parameter is the <em>control_ID</em> which identifies the specific Dial control in the <strong>Dash</strong> app and the second attribute is simply the Dial value.</p>

<p>Once again, run the above code and the <strong>Dash</strong> app. This time, a new "DIAL" messages will be seen on the serial monitor every second.</p>

<p>The next step is to show the Dial values from the messages on a control on the <strong>Dash</strong> app.</p>

<p>In the <strong>Dash</strong> app, tap the <img src="https://dashio.io/wp-content/uploads/2021/07/iot_blue_44.png" width="20"> <strong>All Devices</strong> button, followed by the <img src="https://dashio.io/wp-content/uploads/2021/07/magnifying_glass_44_blue.png" width="20"> <strong>Find New Device</strong> button. Then select the <strong>TCP Discovery</strong> option to show a list of new IoT devices that are TCP mDNS enabled. Your IoT device should be shown in the list. Select your device and from the next menu select <strong>Create Device View</strong>. This will create an empty Device View for your new IoT deivce. You can now add controls to the Device View:</p>

<h3 id="toc_8">Adding Controls to Dash App</h3>

Once you have discovered your IoT device in the **Dash** app and have a **Device View** available for editing, you can add controls to the **Device View**:

<ul>
<li><strong>Start Editing</strong>: Tap the <img src="https://dashio.io/wp-content/uploads/2021/07/pencil_44_blue.png" width="20"> <strong>Edit</strong> button (it not already in editing mode).</li>
<li><strong>Add Dial Control</strong>: Tap the <img src="https://dashio.io/wp-content/uploads/2021/07/add_44_blue.png" width="20"> <strong>Add Control</strong> button and select the Dial control from the list.</li>
<li><strong>Edit Controls</strong>: Tap the Dial control to select it. The <img src="https://dashio.io/wp-content/uploads/2021/07/spanner_44_button.png" width="20"> <strong>Control Settings Menu</strong> button will appear in the middle of the Control. The Control can then be dragged and resized (pinch). Tapping the <img src="https://dashio.io/wp-content/uploads/2021/07/spanner_44_button.png" width="20"> button allows you to edit the Control settings where you can setup the style, colors and other characteristics of your Control. Make sure the <em>Control_ID</em> is set to the same value that is used in the Dial messages (in this case it should be set to "D01").</li>
<li><strong>Quit editing</strong>: Tap the <img src="https://dashio.io/wp-content/uploads/2021/07/pencil_quit_44.png" width="20"> <strong>Edit</strong> button again.</li>
</ul>

<p>The Dial on the Dash app will now show the random Dial values as they arrive.</p>

<p>The next piece of the puzzle to consider is how your IoT device can receive data from the <strong>Dash</strong> app. Lets add a Knob and connect it to the Dial.</p>

<p>In the <strong>Dash</strong> app you will need to add a <strong>Knob</strong> control onto your <strong>Device View</strong>, next to your <strong>Dial</strong> control. Edit the <strong>Knob</strong> to make sure the <strong>Control ID</strong> of the Knob matches what you have used in your Knob messages (in this case it should be "KB01"), then quit edit mode.</p>

Continuing with the TCP example, in the Arduino code we need to respond to messages coming in from a Knob control that we just added to the <strong>Dash</strong> app. To make the changes to your IoT device we add a callback, <code>processIncomingMessage</code>, into the TCP connection with the <code>setCallback</code> function.

```
#include "DashioESP.h"

char WIFI_SSID[] = "yourWiFiSSID";
char WIFI_PASSWORD[] = "yourWiFiPassword";

DashDevice dashDevice("ESP8266_Type");
DashTCP    tcp_con(&dashDevice, true);
DashWiFi   wifi;

int dialValue = 0;

void processIncomingMessage(MessageData *messageData) {
    switch (messageData->control) {
    case knob:
        if (messageData->idStr == "KB01") {
            dialValue = messageData->payloadStr.toFloat();
            String message = dashDevice.getDialMessage("D01", dialValue);
            tcp_con.sendMessage(message);
        }
        break;
    }
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    tcp_con.setCallback(&processIncomingMessage);
    dashDevice.setup(wifi.macAddress(), "Jim Name"); // unique deviceID, and device name
    wifi.attachConnection(&tcp_con);
    wifi.begin(WIFI_SSID, WIFI_PASSWORD);
}

void loop() {
    wifi.run();
}
```

<p>We obtain the Knob value from the message data payload that we receive in the <code>processIncomingMessage</code> function. We then create a Dial message with the value from the Knob and send this back to the <strong>Dash</strong> app. Remember to remove the timer and the associated Dial <code>tcp_con.sendMessage</code> from the <code>loop()</code> function.</p>

<p>When you adjust the Knob on the <strong>Dash</strong> app, a message with the Knob value is sent your IoT device, which returns the Knob value into the Dial control, which you will see on the <strong>Dash</strong> app.</p>

<p>Finally, we should respond to the STATUS message from the <strong>Dash</strong> app. STATUS messages allows the IoT device to send initial conditions for each control to the <strong>Dash</strong> app as soon as a connection becomes active. Once again, we do this from the <code>processIncomingMessage</code> function and our complete code looks like this:</p>

```
#include "DashioESP.h"

char WIFI_SSID[] = "yourWiFiSSID";
char WIFI_PASSWORD[] = "yourWiFiPassword";

DashDevice dashDevice("ESP8266_Type");
DashTCP    tcp_con(&dashDevice, true);
DashWiFi   wifi;

int dialValue = 0;

void processStatus(ConnectionType connectionType) {
    String message((char *)0);
    message.reserve(1024);

    message = dashDevice.getKnobMessage("KB01", dialValue);
    message += dashDevice.getDialMessage("D01", dialValue);

    tcp_con.sendMessage(message);
}

void processIncomingMessage(MessageData *messageData) {
    switch (messageData->control) {
    case status:
        processStatus(messageData->connectionType);
        break;
    case knob:
        if (messageData->idStr == "KB01") {
            dialValue = messageData->payloadStr.toFloat();
            String message = dashDevice.getDialMessage("D01", dialValue);
            tcp_con.sendMessage(message);
        }
        break;
    }
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    tcp_con.setCallback(&processIncomingMessage);
    dashDevice.setup(wifi.macAddress(), "Jim Name"); // unique deviceID, and device name
    wifi.attachConnection(&tcp_con);
    wifi.begin(WIFI_SSID, WIFI_PASSWORD);
}

void loop() {
    wifi.run();
}
```

<h3 id="toc_9">MQTT Basics</h3>

<p>Now lets look at a MQTT connection. We also need the WiFi for MQTT.</p>

```
#include "DashioESP.h"

// WiFi
char WIFI_SSID[] = "yourWiFiSSID";
char WIFI_PASSWORD[] = "yourWiFiPassword";

// MQTT
char MQTT_USER[] = "yourMQTTuserName";
char MQTT_PASSWORD[] = "yourMQTTpassword";

DashDevice dashDevice("ESP8266_Type");
DashMQTT mqtt_con(&dashDevice, true, true);
DashWiFi wifi;

void setup() {
    Serial.begin(115200);

    dashDevice.setup(wifi.macAddress(), "Bob Name"); // unique deviceID, and device name
    mqtt_con.setup(MQTT_USER, MQTT_PASSWORD);
    wifi.attachConnection(&mqtt_con);
    wifi.begin(WIFI_SSID, WIFI_PASSWORD);
}

void loop() {
    wifi.run();
}
```

<p>Again we create a device and this time a MQTT connection, with the device being an attribute to the connection. The second attribute of the MQTT connection enables a push notification to be sent to your mobile phone when the IoT device reboots (<strong>Dash</strong> MQTT broker only). The final attribute enables the received and transmitted messages to be printed to the serial monitor.</p>

<p>In the <code>setup()</code> function we setup the device with two parameters. These parameters describe the device to the <strong>Dash</strong> app to allow it uniquely identify each device. The wifi object supplies the mac address to the device, to be used as a unique <em>device_ID</em>.</p>

<p>For MQTT connections, the device must be setup before we begin the wifi.</p>

<p>The MQTT connection setup requires the MQTT broker <em>username</em> and <em>password</em>, supplied in the <code>mqtt_con.setup(MQTT_USER, MQTT_PASSWORD);</code> method.</p>

<p>We then attach the MQTT connection to the <code>wifi</code> with <code>wifi.attachConnection(&mqtt_con);</code>. This enables the <code>wifi</code> object to control the MQTT connection from here on in.</p>

<p>Finally, we start the WiFi with <code>wifi.begin(WIFI_SSID, WIFI_PASSWORD);</code>, including parameters for the SSID and password.</p>

<p><code>wifi.run();</code> is used to regularly check that the WiFi is connected and to manage the attached connection. </p>

<p>This device is discoverable by the <strong>Dash</strong> app.</p>

<p>In the code above, make sure you replace <strong>yourWiFiSSID</strong> with your WiFi SSID, <strong>yourWiFiPassword</strong> with your WiFI password, <strong>yourMQTTuserName</strong> with you <strong>dash</strong> account username and <strong>yourMQTTpassword</strong> with your <strong>dash</strong> account password. Setup the Arduino IDE serial monitor with 115200 baud and run the code. You will see messages on the Arduino serial monitor, and after a few attempts, your IoT device should connect to your WiFi and then connect to the <strong>dash</strong> MQTT server.</p>

<p>Message processing for a MQTT connection is done exactly the same way as for the TCP connection, by adding the <code>processIncomingMessage</code> callback into the MQTT connection during <code>setup()</code> function. Our finished code, with Dial and Knob added is:</p>

```
#include "DashioESP.h"

// WiFi
char WIFI_SSID[] = "yourWiFiSSID";
char WIFI_PASSWORD[] = "yourWiFiPassword";

// MQTT
char MQTT_USER[] = "yourMQTTuserName";
char MQTT_PASSWORD[] = "yourMQTTpassword";

DashDevice dashDevice("ESP8266_Type");
DashMQTT   mqtt_con(&dashDevice, true, true);
DashWiFi   wifi;

int dialValue = 0;

void processStatus(ConnectionType connectionType) {
    String message((char *)0);
    message.reserve(1024);

    message = dashDevice.getKnobMessage("KB01", dialValue);
    message += dashDevice.getDialMessage("D01", dialValue);

    mqtt_con.sendMessage(message);
}

void processIncomingMessage(MessageData *messageData) {
    switch (messageData->control) {
    case status:
        processStatus(messageData->connectionType);
        break;
    case knob:
        if (messageData->idStr == "KB01") {
            dialValue = messageData->payloadStr.toFloat();
            String message = dashDevice.getDialMessage("D01", dialValue);
            mqtt_con.sendMessage(message);
        }
        break;
    }
}

void setup() {
    Serial.begin(115200);

    dashDevice.setup(wifi.macAddress(), "Bob Name"); // unique deviceID, and device name
    mqtt_con.setup(MQTT_USER, MQTT_PASSWORD);
    mqtt_con.setCallback(&processIncomingMessage);
    wifi.attachConnection(&mqtt_con);
    wifi.begin(WIFI_SSID, WIFI_PASSWORD);
}

void loop() {
    wifi.run();
}
```

<p>Run the <strong>Dash</strong> app once more. Tap the <img src="https://dashio.io/wp-content/uploads/2021/07/iot_blue_44.png" width="20"> <strong>All Devices</strong> button, followed by the <img src="https://dashio.io/wp-content/uploads/2021/07/magnifying_glass_44_blue.png" width="20"> <strong>Find New Devices</strong> button. Then select the <strong>My Devices On Dash</strong> option to show a list of new IoT devices that have announced themselves to the <strong>dash</strong> server. Your device will be shown (named "Bob Name"). Select your IoT device to add the MQTT connection to your existing IoT device with the TCP connection.</p>

For the next step, you will need to setup your **Dash** app with Dial and Knob controls, as described in the section above **Adding Controls to Dash App**.

<p>When you adjust the Knob on the <strong>Dash</strong> app, a message with the Knob value is sent your IoT device, which returns the Knob value into the Dial control, which you will see on the <strong>Dash</strong> app.</p>

<p>For MQTT connections operating through the <strong>dash</strong> MQTT broker, a service is available for sending <strong>Alarms</strong> (push notifications) to your mobile device. These are handy for sending messages or alarms to the user when they are not currently using the <strong>Dash</strong> app. To send an alarm, simply call the MQTT connection&#39;s <code>sendMessage</code> method with two attrbutes; the first being any message String and the second having <strong>alarm_topic</strong> specified.</p>

<div><pre><code class="language-none">mqtt_con.sendMessage("message", alarm_topic);</code></pre></div>

<h2 id="toc_10">Combining TCP and MQTT</h2>

<p>The ESP8266 is capable of operating with both TCP and MQTT connections. It&#39;s relativelty easy to combine to code for both connections in one ESP8266 IoT device where each connection calls a common processIncomingMessage callback.</p>

When combining connections, it is useful to create a generic ```sendMessage``` function to reply to an incoming message for a specific connection type. This prevents sending unnecessary reply messages to all connections:

```
void sendMessage(ConnectionType connectionType, const String& message) {
    if (connectionType == TCP_CONN) {
        tcp_con.sendMessage(message);
    } else {
        mqtt_con.sendMessage(message);
    }
}
```

<img src="https://dashio.io/wp-content/uploads/2022/09/attention_44_yellow.png" width="20"> <strong>Troubleshooting:</strong> When you have debugged your code, make sure to set your debug level back to **None** to minimise any effects of the debug mechanism on you code performance.

This is just the beginning and there is a lot more we can do. Take a look at the examples in the library to see more details.

<h2 id="toc_11">Layout Configuration</h2>

<p><strong>Layout configuration</strong> allows the IoT device to hold a copy of the complete layout of the device as it should appear on the <strong>Dash</strong> app. It includes all infomration for the device, controls (size, colour, style etc.), device views and connections.</p>

<p>When the <strong>Dash</strong> app discovers a new IoT device, it will download the Layout Configuration from the IoT device to display the device controls they way they have been designed by the developer. This is particularly useful developing commercial products and distributing your IoT devices to other users.</p>

<p>To include the Layout Configuration in your IoT device, simply follow these steps:</p>

<ul>
<li><strong>Design your layout</strong> in the <strong>Dash</strong> app and include all controls and connections that you need in your layout.</li>
<li><strong>Export the layout</strong>: Tap on the <strong>Device</strong> button, then tap the <strong>Export Layout</strong> button.</li>
<li><strong>Select the provisioning setup</strong> that you want (see below for provisioning details) and tap the <strong>Export</strong> button. The Layout Configuration will be emailed to you.</li>
<li><strong>Copy and paste</strong> the C64 configuration text from the email into your Arduino code, assigning it to a pointer to store the text in program memory. Your C64 configuration text will be different to that shown below.</li>
<li><strong>Add the pointer</strong> to the C64 configuration text (configC64Str) as a second attribute to the DashDevice object.</li>
<li><strong>Add the Layout Config Revision</strong> integer (CONFIG_REV) as a third attribute to the DashDevice object.</li>
</ul>

```
const char configC64Str[] PROGMEM =
"lVNNk5pAEP0r1hxyolKKcU28CShagiDOYlKpHFiZyEScIcPgx1r+9/Tw4ZqsOVhcmu7X3dPvdZ+RYzho8P2HhmZzzwDrjIqCxmiA"
"Zj1jX2yKpwWLwpVr73zfLsIN0lAuTykBwNwL3KEDjjVnUvB0aqkso91RGMJij6UnjwUkJVEOeCkKoqFddESDTrutoSwShMkyyQrL"
"JEHiMEoLwPYhLqks28wYf4FgQugmkUEkKUeD9kf9qUb4PKfgY4DEnq9aJ/zgUuaqRlXPU4O5Zvc+wdfT0BZqmzzlQj2CkKzlU7aF"
"GjGN0jFPU37Iy/Z1IeVu4N+ICgP2QGOZ3FTW0PFdP13v6/0uTE/hne0LsL10rKDiHY++4sqyprXLHfq1MZo/V5bj2ZUxDK3KWDom"
"rvGmUxtWGK5KDRuNDC+wRsFSiSRFCpyMQaslfYVYFzjeCBrDQMWO5Wig64o8UKXyNEOzYneFdP4Wu9ZNlTa4iIloyFklVJImst2w"
"uAkYSt7K/18sFhHLy+1Yn4At1bJJjtZKnWYzLJCj9aFVL4jKxypi8OOddk3oXXXFQQBCAyPdG+gbUZ3PGqIw9jzaqa7PGbqUYg2d"
"23P50g7Fb8OYYNueWklCR9ns13Fxcy7GMIC/TJA1zct97f5DZsnl4/ehWHjkPsrdpumVf1sQwtC9K+n01Acv4ZTJN3Gvmw+b8ULE"
"TQNzNMej4O4NPnIlBaMS5EDo9mDMSVCfCbYDf1KZBsbzxrLrmzHHcClnFJM9XZMlkUWmRjd9zV1gXL3aKmMhJYd6p39uArIH83L5"
"Aw==";

DashDevice    dashDevice(DEVICE_TYPE, configC64Str, CONFIG_REV);
```

Here is our Knob and Dial example, with TCP and MQTT connections and Layout Configuration added:

```
#include "DashioESP.h"

const char configC64Str[] PROGMEM =
"lVNNk5pAEP0r1hxyolKKcU28CShagiDOYlKpHFiZyEScIcPgx1r+9/Tw4ZqsOVhcmu7X3dPvdZ+RYzho8P2HhmZzzwDrjIqCxmiA"
"Zj1jX2yKpwWLwpVr73zfLsIN0lAuTykBwNwL3KEDjjVnUvB0aqkso91RGMJij6UnjwUkJVEOeCkKoqFddESDTrutoSwShMkyyQrL"
"JEHiMEoLwPYhLqks28wYf4FgQugmkUEkKUeD9kf9qUb4PKfgY4DEnq9aJ/zgUuaqRlXPU4O5Zvc+wdfT0BZqmzzlQj2CkKzlU7aF"
"GjGN0jFPU37Iy/Z1IeVu4N+ICgP2QGOZ3FTW0PFdP13v6/0uTE/hne0LsL10rKDiHY++4sqyprXLHfq1MZo/V5bj2ZUxDK3KWDom"
"rvGmUxtWGK5KDRuNDC+wRsFSiSRFCpyMQaslfYVYFzjeCBrDQMWO5Wig64o8UKXyNEOzYneFdP4Wu9ZNlTa4iIloyFklVJImst2w"
"uAkYSt7K/18sFhHLy+1Yn4At1bJJjtZKnWYzLJCj9aFVL4jKxypi8OOddk3oXXXFQQBCAyPdG+gbUZ3PGqIw9jzaqa7PGbqUYg2d"
"23P50g7Fb8OYYNueWklCR9ns13Fxcy7GMIC/TJA1zct97f5DZsnl4/ehWHjkPsrdpumVf1sQwtC9K+n01Acv4ZTJN3Gvmw+b8ULE"
"TQNzNMej4O4NPnIlBaMS5EDo9mDMSVCfCbYDf1KZBsbzxrLrmzHHcClnFJM9XZMlkUWmRjd9zV1gXL3aKmMhJYd6p39uArIH83L5"
"Aw==";

char WIFI_SSID[] = "yourWiFiSSID";
char WIFI_PASSWORD[] = "yourWiFiPassword";
char MQTT_USER[] = "yourMQTTuserName";
char MQTT_PASSWORD[] = "yourMQTTpassword";

DashDevice dashDevice("ESP8266_Type", configC64Str, 1);
DashTCP    tcp_con(&dashDevice, true);
DashMQTT   mqtt_con(&dashDevice, true, true);
DashWiFi   wifi;

int dialValue = 0;

void sendMessage(ConnectionType connectionType, const String& message) {
    if (connectionType == TCP_CONN) {
        tcp_con.sendMessage(message);
    } else {
        mqtt_con.sendMessage(message);
    }
}

void processStatus(ConnectionType connectionType) {
    String message((char *)0);
    message.reserve(1024);

    message = dashDevice.getKnobMessage("KB01", dialValue);
    message += dashDevice.getDialMessage("D01", dialValue);

    sendMessage(connectionType, message);
}

void processIncomingMessage(MessageData *messageData) {
    switch (messageData->control) {
    case status:
        processStatus(messageData->connectionType);
        break;
    case knob:
        if (messageData->idStr == "KB01") {
            dialValue = messageData->payloadStr.toFloat();
            String message = dashDevice.getDialMessage("D01", dialValue);
            sendMessage(messageData->connectionType, message);
        }
        break;
    }
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    dashDevice.setup(wifi.macAddress(), "Jim Name"); // unique deviceID, and device name

    tcp_con.setCallback(&processIncomingMessage);
    wifi.attachConnection(&tcp_con);
    
    mqtt_con.setup(MQTT_USER, MQTT_PASSWORD);
    mqtt_con.setCallback(&processIncomingMessage);
    wifi.attachConnection(&mqtt_con);

    wifi.begin(WIFI_SSID, WIFI_PASSWORD);
}

void loop() {
    wifi.run();
}
```

<h2 id="toc_12">Provisioning</h2>

Provisioning is the method by which the **Dash** app can send user credentials and other setup information to the IoT device and the IoT device will store this data in non-volatile memory.

Details of the provisioning functionality available in an IoT device are contained in the Layout Configuration and must be setup when exporting the Layout Configuration from the <strong>Dash</strong> app.

A provisioning library is available within the Dash library for the ESP32 (DashioProvisionESP.h and DashioProvisionESP.cpp) which stores the following provisioning data to non-volatile memory:

- Device Name
- WiFi SSID
- WiFI Password
- Dash MQTT broker User Name
- Dash MQTT broker Password

The library exposes the following structure to contain the provisioning data:

```
struct DeviceData {
    char deviceName[32];
    char wifiSSID[32];
    char wifiPassword[63];
    char dashUserName[32];
    char dashPassword[32];
    char saved;
};
```

Use the provisioning library as follows:

**Include the library**

```
#include "DashioProvisionESP.h"
```

**Create a provisioning object**

```
DashProvision dashProvision(&dashDevice);
```

**Setup provisioning** in the ```setup()``` function. For the TCP connection, we assign the provisioning information to the WiFi object  or, for an MQTT connection, we assign the provisioning information to the WiFI object and MQTT connection object:

```
// setup
DeviceData defaultDeviceData = {DEVICE_NAME, WIFI_SSID, WIFI_PASSWORD, MQTT_USER, MQTT_PASSWORD};
dashProvision.load(&defaultDeviceData, &onProvisionCallback);

dashDevice.setup(wifi.macAddress());
    
mqtt_con.setup(dashProvision.dashUserName, dashProvision.dashPassword);
mqtt_con.setCallback(&processIncomingMessage);
    
wifi.attachConnection(&mqtt_con);
wifi.begin(dashProvision.wifiSSID, dashProvision.wifiPassword);
```

Here we update the provisioning object with the default provisioning data (using the <code>DeviceData</code> structure), then load the provisioning data. If the provisioning data has never been saved, then the the default provisionig data will be used. We also include a provision callback (discussed later).

**Add provisioning process message** in the ```processIncomingMessage``` callback from the example above becomes:

```
void processIncomingMessage(MessageData * messageData) {
    switch (messageData->control) {
    case status:
        processStatus(messageData->connectionType);
        break;
    case knob:
        if (messageData->idStr == "KB01") {
            dialValue = messageData->payloadStr.toFloat();
            String message = dashDevice.getDialMessage("D01", dialValue);
            sendMessage(messageData->connectionType, message);
        }
        break;
    default:
        dashProvision.processMessage(messageData);
        break;
    }
}
```

Your IoT device will let the <strong>Dash</strong> app know what provisioning services are available when it down loads the <strong>layout configuration</strong>.

And finally, here is the <code>onProvisionCallback</code>:

```
void onProvisionCallback(ConnectionType connectionType, const String& message, bool commsChanged) {
    sendMessage(connectionType, message);

    if (commsChanged) {
        mqtt_con.setup(dashProvision.dashUserName, dashProvision.dashPassword);
        wifi.begin(dashProvision.wifiSSID, dashProvision.wifiPassword);
    } else { // Must be a name change
        mqtt_con.sendWhoAnnounce(); // Update to announce topic with new name
    }
}
```

<The callback has two purposes. Firstly, we send a message back to the <strong>Dash</strong> app to let it know that provisioning is complete. And secondly, if the WiFi or MQTT connection credentials have changed, we need to restart the WiFi or MQTT connection. In the above example we call <code>mqtt_con.setup</code> to update the MQTT connection with the new username and password and <code>wifi.begin</code> to update wifi with the new SSID and password. Alternatively, you could reboot the processor for a fresh start which would also update the MQTT connection and WiFi credentials.

To provision your device, run the <strong>Dash</strong> app and tap the <img src="https://dashio.io/wp-content/uploads/2021/07/iot_blue_44.png" width="20"> <strong>All Devices</strong> button and select your device. Tap the <img src="https://dashio.io/wp-content/uploads/2022/09/key_44_blue.png" width="20"> <strong>Provisioning</strong> button and follow the instructions.

The final provisioning feature is the ability to "Reset" your IoT device. The **Dash** app provisioning menu also includes a "Reset Device" button. In your IoT device, you will receive an incoming message of ControlType = ***resetDevice*** which you can manage in your ```processIncomingMessage``` callback as shown in the following example:

```
void processIncomingMessage(MessageData * messageData) {
    switch (messageData->control) {
    case status:
        processStatus(messageData->connectionType);
        break;
    case knob:
        if (messageData->idStr == "KB01") {
            dialValue = messageData->payloadStr.toFloat();
            String message = dashDevice.getDialMessage("D01", dialValue);
            sendMessage(messageData->connectionType, message);
        }
        break;
    case resetDevice:
        sendMessage(messageData->connectionType, dashDevice.getResetDeviceMessage());
        delay(2000); // Make sure the above message is sent before resetting
        ESP.restart();
        break;
    default:
        dashProvision.processMessage(messageData);
        break;
    }
}
```

<h2 id="toc_13">SoftAP Provisioning</h2>

<p>SoftAP enables the ESP8266 to be made into a wireless access point. This is particularly useful for provisioning WiFi, TCP and MQTT services because the ESP8266 does not contain BLE functionality (BLE is very useful for provisioning).</p>

<p>The SoftAP functionality is included in the DashioESP library and can be implemented follows:</p>

```
#include <DashioESP.h>

char DEVICE_TYPE[] = "ESP8266_DashIO";
char DEVICE_NAME[] = "DashIO8266";

#define SOFT_AP_PWD    "" // Can be empty i.e. "" or must be a VALID password

DashDevice    dashDevice(DEVICE_TYPE);
DashSoftAP    soft_AP;
DashTCP       tcp_con(&dashDevice, true);

void setup() {
    Serial.begin(115200);
    delay(1000);

    dashDevice.setup(WiFi.macAddress(), DEVICE_NAME); // Get unique deviceID
    Serial.print(F("Device ID: "));
    Serial.println(dashDevice.deviceID);

    soft_AP.attachConnection(&tcp_con);

    if (!soft_AP.begin(SOFT_AP_PWD)) {
        Serial.println(F("Failed to start soft AP"));
    }
}

void loop() {
    soft_AP.run();
}
```

<p>The <code>DashSoftAP</code> class behaves in a similar manner to the <code>DashWiFi</code> class. You simply attach a TCP connection to it and it will manage the connection to allow the <strong>Dash</strong> app to connect to it.</p>

This device is discoverable by the <strong>Dash</strong> app. However, you will need to connect you phone or tablet WiFi to the SoftAP WiFi SSID which is called "Dash_Provision". You will then be able to discover your IoT device with the normal TCP discovery functionality within the <strong>Dash</strong> app (<em>Devices/Find New Devices/TCP Discovery</em>). After you have provisioned your device, connect your phone or tablet back to you normal WiFi.

For referency only, the SoftAP IP address is: **192.168.68.100** and uses port **55892** for discovery only on the <strong>Dash</strong> app. This address and port is independent of the address and port for the TCP connection attached to the SoftAP.

<p>Take a look in the library example for <strong>DashIO_ESP8266_softAP</strong>, which incorporates full provisioning of WiFi, TCP and MQTT connections. A push button is used to switch to the SoftAP to enable provisioning. After the phone or tablet disconnects from the SoftAP, the IoT device switches back to the WiFi that you have provisioned.</p>

<h1 id="toc_14">Jump In and Build Your Own IoT Device</h1>

When you are ready to create your own IoT device, the Dash Arduino C++ Library will provide you with more details about what you need to know:

<a href="https://dashio.io/arduino-library/">https://dashio.io/arduino-library/</a>