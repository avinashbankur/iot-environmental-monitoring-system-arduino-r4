#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <WiFiS3.h>
#include <PubSubClient.h>

// ---------------- WiFi & MQTT Settings ----------------
char ssid[] = ""; // Wi-Fi SSID
char password[] = ""; // Wi-Fi Password
const char* mqtt_server = "test.mosquitto.org"; // Public MQTT broker

WiFiClient espClient;
PubSubClient client(espClient);

// ---------------- LCD Setup ----------------
LiquidCrystal_I2C lcd(0x27, 16, 2); // Change to 0x3F if LCD not detected

// ---------------- DHT11 Setup ----------------
#define DHTPIN 4
uint8_t data[5]; // 5 bytes from DHT11

// ---------------- MQ135 Setup ----------------
#define MQ135_PIN A0

// ---------------- Function Declarations ----------------
void setup_wifi();
void reconnect();
bool readDHT11();

// ---------------- Setup ----------------
void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\nStarting IoT Env Monitor...");

  // LCD initialization
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("System Booting...");
  delay(2000);

  // Wi-Fi connection
  setup_wifi();

  // MQTT setup
  client.setServer(mqtt_server, 1883);

  // MQ135 pin
  pinMode(MQ135_PIN, INPUT);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("WiFi OK + MQTT");
  lcd.setCursor(0, 1);
  lcd.print("Initializing...");
  delay(2000);
}

// ---------------- Loop ----------------
void loop() {
  if (!client.connected()) reconnect();
  client.loop();

  // Read DHT11 sensor
  if (readDHT11()) {
    int temperature = data[2]; // Temperature
    int humidity = data[0];    // Humidity
    int airQuality = analogRead(MQ135_PIN);

    // Display on LCD
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("T:");
    lcd.print(temperature);
    lcd.print((char)223); // Degree symbol
    lcd.print("C H:");
    lcd.print(humidity);
    lcd.print("%");

    lcd.setCursor(0, 1);
    lcd.print("Air:");
    lcd.print(airQuality);

    // Publish JSON payload
    char payload[60];
    sprintf(payload, "{\"temp\":%d,\"hum\":%d,\"air\":%d}", temperature, humidity, airQuality);
    client.publish("iot/env_monitor", payload);

    Serial.println(payload);
  } else {
    Serial.println("DHT11 read failed");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("DHT11 Error!");
  }

  delay(2000);
}

// ---------------- Wi-Fi Connection ----------------
void setup_wifi() {
  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 15000) {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi connected!");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nWiFi failed! Check credentials or router.");
  }
}

// ---------------- MQTT Reconnect ----------------
void reconnect() {
  while (!client.connected()) {
    Serial.print("Connecting to MQTT...");
    if (client.connect("ArduinoUNO_R4")) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" retrying in 5s");
      delay(5000);
    }
  }
}

// ---------------- DHT11 Manual Read ----------------
bool readDHT11() {
  uint8_t laststate = HIGH, counter = 0, j = 0, i;
  for (i = 0; i < 5; i++) data[i] = 0;

  // Start signal
  pinMode(DHTPIN, OUTPUT);
  digitalWrite(DHTPIN, LOW);
  delay(18);
  digitalWrite(DHTPIN, HIGH);
  delayMicroseconds(40);
  pinMode(DHTPIN, INPUT);

  // Read the data (low-level bit timing)
  for (i = 0; i < 85; i++) {
    counter = 0;
    while (digitalRead(DHTPIN) == laststate) {
      counter++;
      delayMicroseconds(1);
      if (counter == 255) break;
    }
    laststate = digitalRead(DHTPIN);
    if (i >= 4 && i % 2 == 0) {
      data[j / 8] <<= 1;
      if (counter > 16) data[j / 8] |= 1;
      j++;
    }
  }

  if (j >= 40) {
    uint8_t sum = data[0] + data[1] + data[2] + data[3];
    if (data[4] == (sum & 0xFF)) return true;
  }
  return false;
}
