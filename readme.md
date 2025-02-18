
# Tank Monitoring System with ESP32 and MQTT 🚰📡

## Description 📜
This project uses an **ESP32** microcontroller to measure the **tank level** with an **ultrasonic sensor** and transmit the values via **WiFi** and **MQTT**. The system features a **captive portal** that allows easy configuration of **WiFi** and **MQTT** settings, and it regularly sends tank level data, the **last update time**, and the **WiFi status** via MQTT.

## Features ✨
- **WiFi and MQTT setup** via a captive portal 🌐
- **Tank level measurement** based on ultrasonic technology 🏞️
- **MQTT communication** for transmitting tank status, last updates, and WiFi connection 📡
- **Saving configurations** (WiFi and MQTT) in internal memory 🔒
- **NTP time synchronization** for accurate timestamps ⏱️
- **Automatic reconnection** on connection drops 🌐

## Installation 🛠️

### 1. Flash the ESP32
- Install **Arduino IDE** or use **PlatformIO**.
- Make sure you have the necessary libraries for your project:
  - `WiFi.h`
  - `WebServer.h`
  - `Preferences.h`
  - `PubSubClient.h`
  - `TimeLib.h`
  - `NTPClient.h`
  - `WiFiUdp.h`
  - `Ultrasonic.h`

### 2. Configuration
- **WiFi**: When first powered on, the ESP32 starts in **Access Point (AP) mode**.
- **MQTT**: Enter the MQTT broker address and credentials via the captive portal.

The captive portal will automatically open in the browser when you connect to the ESP32, allowing you to input **WiFi** and **MQTT** data.

### 3. MQTT Topics 📨
- `tank/level`: Tank level in percentage.
- `tank/status`: Tank status (0 = empty).
- `tank/lastupdate`: Time of the last update.
- `tank/wificonnect`: Time of the last WiFi connection.

### 4. Home Assistant Integration 🏠🤖
Automations in **Home Assistant** can access the MQTT data. For example, a notification when the tank drops below 20%.

#### Example Automation: Tank Level Below 20%
```yaml
alias: "[Tank Level] - Low Level Notification"
description: "Sends a notification when the tank level drops below 20%"
trigger:
  - platform: mqtt
    topic: tank/level
    value_template: "{{ value|float < 20 }}"
action:
  - service: notify.notify
    data:
      message: "The tank level has dropped below 20%!"
mode: single
```

## Wiring 🔌
- **Ultrasonic sensor**: Connect the **Trigger** pin to **GPIO4** and the **Echo** pin to **GPIO18** of the ESP32.
- The ESP32 should be powered by a stable **5V power supply**.

## Captive Portal HTML Page 🖥️
The captive portal allows configuring **WiFi** and **MQTT** data directly via an HTML interface. The page shows fields for:
- WiFi SSID and password
- MQTT broker, username, and password
- Tank level (Empty Level and Full Level)

```html
<!DOCTYPE html>
<html>
<head>
  <title>WiFi and MQTT Setup</title>
  <style>
    body { font-family: Arial; text-align: center; margin-top: 50px; }
    input { padding: 10px; margin: 10px; width: 80%; }
    button { padding: 10px 20px; background-color: #4CAF50; color: white; border: none; cursor: pointer; }
    button:hover { background-color: #45a049; }
  </style>
</head>
<body>
  <h1>WiFi and MQTT Setup</h1>
  <form action="/set_wifi" method="POST">
    <h2>WiFi Settings</h2>
    <input type="text" name="ssid" placeholder="WiFi SSID" required><br>
    <input type="password" name="wifipassword" placeholder="WiFi Password" required><br>
    <h2>MQTT Settings</h2>
    <input type="text" name="mqtt_broker" placeholder="MQTT Broker IP" required><br>
    <input type="text" name="mqtt_username" placeholder="MQTT Username" required><br>
    <input type="password" name="mqtt_password" placeholder="MQTT Password" required><br>
    <button type="submit">Save</button>
  </form>
  <br>
  <hr>
  <h2>Tank Level Settings</h2>
  <form action="/set_tank_settings" method="POST">
    <label for="empty_level">Empty Level (cm):</label><br>
    <input type="text" name="empty_level" value="30" required><br>
    <label for="full_level">Full Level (cm):</label><br>
    <input type="text" name="full_level" value="5" required><br>
    <button type="submit">Save</button>
  </form>
</body>
</html>
```

## License 📝
This project is licensed under the **MIT License**. For more details, please see the [LICENSE](LICENSE) file.

## Acknowledgements 🙏
- **ESP32**: The microcontroller for WiFi and MQTT.
- **Ultrasonic Sensor**: For precise tank level measurements.
- **PubSubClient**: MQTT client library.
- **Arduino Community**: For support and libraries.
- **Home Assistant**: For integration and automation.

## Contact 📬
For questions or support, please contact via **GitHub Issues**.

**Aydin Voelk**  
📱 [+49 171 29 29 080](tel:+491712929080)  
📧 [Aydin.Voelk@Voelk-IT.de](mailto:Aydin.Voelk@Voelk-IT.de)  
💻 [GitHub](https://github.com/Voelk-IT/) | 🌐 [Website](https://voelk-it.de)


### 💙 Support my Work ♥  
[![Donate](https://d-is.de/donate.png)](https://paypal.me/GinaSophieVoelk?country.x=DE&locale.x=de_DE)
