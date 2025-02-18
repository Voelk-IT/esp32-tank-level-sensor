#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>
#include <PubSubClient.h>
#include <TimeLib.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <Ultrasonic.h>

// Default values for WiFi and MQTT
char ssid[50] = "Setup_Hotspot";  // SSID of the access point
char password[50] = "SetupPassword";  // Password of the access point

char mqtt_broker[50] = "192.168.1.100";  // Default MQTT broker
char mqtt_username[50] = "mqtt";  // Default MQTT username
char mqtt_password[50] = "mqtt";  // Default MQTT password
char mqtt_topic_level[50] = "tank/level";  // Tank level topic
char mqtt_topic_status[50] = "tank/status";  // Status topic
char mqtt_topic_lastupdate[50] = "tank/lastupdate";  // Last update topic
char mqtt_topic_wificonnect[50] = "tank/wificonnect";  // WiFi connect topic

// Objects for WiFi, WebServer, and MQTT
WebServer server(80);
WiFiClient espClient;
PubSubClient client(espClient);

// Memory for settings
Preferences preferences;

// NTP client for time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 0, 60000);  // Get time from NTP server

// Ultrasonic sensor for tank level (Trigger and Echo pins)
const int triggerPin = 4;
const int echoPin = 18;
Ultrasonic ultrasonic(triggerPin, echoPin);

// HTML page for the captive portal
const char index_html[] PROGMEM = R"rawliteral(
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
)rawliteral";

// Setup for the captive portal
void setupCaptivePortal() {
  server.on("/", HTTP_GET, []() {
    server.send(200, "text/html", index_html);
  });

  server.on("/set_wifi", HTTP_POST, []() {
    String ssid_input = server.arg("ssid");
    String wifi_password_input = server.arg("wifipassword");
    String mqtt_broker_input = server.arg("mqtt_broker");
    String mqtt_username_input = server.arg("mqtt_username");
    String mqtt_password_input = server.arg("mqtt_password");

    preferences.putString("ssid", ssid_input);
    preferences.putString("wifi_password", wifi_password_input);
    preferences.putString("mqtt_broker", mqtt_broker_input);
    preferences.putString("mqtt_username", mqtt_username_input);
    preferences.putString("mqtt_password", mqtt_password_input);

    server.send(200, "text/html", "<h1>Settings Saved!</h1><p>Device is restarting...</p>");
    delay(2000);
    ESP.restart();
  });

  server.on("/set_tank_settings", HTTP_POST, []() {
    String empty_level_input = server.arg("empty_level");
    String full_level_input = server.arg("full_level");

    preferences.putFloat("empty_level", empty_level_input.toFloat());
    preferences.putFloat("full_level", full_level_input.toFloat());

    server.send(200, "text/html", "<h1>Tank Level Settings Saved!</h1><p>Values updated.</p>");
  });

  server.begin();  // Start the web server
  Serial.println("Web server started.");
}

// Connect to MQTT
void connectToMQTT() {
  client.setServer(mqtt_broker, 1883);
  while (!client.connected()) {
    Serial.println("Connecting to MQTT broker...");
    if (client.connect("ESP32", mqtt_username, mqtt_password)) {
      Serial.println("Connected!");
      client.publish(mqtt_topic_status, "0");  // Tank Status (0 = empty)
      client.publish(mqtt_topic_wificonnect, getCurrentTime().c_str());  // Time of WiFi connection
    } else {
      Serial.print("Error, rc=");
      Serial.println(client.state());
      delay(2000);
    }
  }
}

// Connect to WiFi
void connectToWiFi() {
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }

  Serial.println("Connected to WiFi!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  client.publish(mqtt_topic_wificonnect, getCurrentTime().c_str());  // Time of WiFi connection
}

// Get the current time (date and time)
String getCurrentTime() {
  timeClient.update();
  return timeClient.getFormattedTime();
}

// Calculate the tank level based on the distance
float getTankLevel() {
  long distance = ultrasonic.read();
  float empty_level = preferences.getFloat("empty_level", 30.0);  // Default value: 30 cm
  float full_level = preferences.getFloat("full_level", 5.0);    // Default value: 5 cm
  float fillPercentage = 100.0 - ((distance - full_level) / (empty_level - full_level) * 100.0);
  fillPercentage = constrain(fillPercentage, 0.0, 100.0);  // Limit to 0-100%
  return fillPercentage;
}

// Setup function
void setup() {
  Serial.begin(115200);
  preferences.begin("mqtt_wifi", false);

  String saved_ssid = preferences.getString("ssid", "");
  String saved_wifi_password = preferences.getString("wifi_password", "");
  String saved_mqtt_broker = preferences.getString("mqtt_broker", "");
  String saved_mqtt_username = preferences.getString("mqtt_username", "");
  String saved_mqtt_password = preferences.getString("mqtt_password", "");

  if (saved_ssid.length() > 0) {
    saved_ssid.toCharArray(ssid, 50);
    saved_wifi_password.toCharArray(password, 50);
    saved_mqtt_broker.toCharArray(mqtt_broker, 50);
    saved_mqtt_username.toCharArray(mqtt_username, 50);
    saved_mqtt_password.toCharArray(mqtt_password, 50);
    Serial.println("WiFi and MQTT data loaded from memory.");
  } else {
    Serial.println("No saved WiFi or MQTT data found.");
  }

  if (saved_ssid.length() == 0 || saved_mqtt_broker.length() == 0) {
    WiFi.softAP(ssid, password);
    Serial.println("Access point started.");
    Serial.print("IP Address: ");
    Serial.println(WiFi.softAPIP());
    setupCaptivePortal();
    while (true) {
      server.handleClient();  // Wait for user requests
    }
  } else {
    connectToWiFi();
    connectToMQTT();
  }
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Lost WiFi connection. Trying again...");
    connectToWiFi();
  }

  if (!client.connected()) {
    connectToMQTT();
  }

  client.loop();

  // Measure tank level and send via MQTT
  float tankLevel = getTankLevel();
  Serial.println("Tank Level: " + String(tankLevel) + "%");
  client.publish(mqtt_topic_level, String(tankLevel).c_str());

  // Send date and time of last update
  client.publish(mqtt_topic_lastupdate, getCurrentTime().c_str());

  delay(1000);  // Wait for 1 second
}
