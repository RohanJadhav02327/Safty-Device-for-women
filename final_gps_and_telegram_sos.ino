#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <SoftwareSerial.h>
#include <TinyGPS++.h>

// ==== WiFi Configuration ====
#define WIFI_SSID "Rohan"
#define WIFI_PASSWORD "#Rohan@308$"

// ==== Telegram Configuration ====
#define BOT_TOKEN "8364987130:AAEPdKrdH31FYuAJTL6kwnB5LAiPSLnTHAs"
#define CHAT_ID "8105694686"   // Your Telegram Chat ID

// ==== Pin Configuration ====
#define BUTTON_PIN D1   // Button input (connect to GND)
#define LED_PIN D2      // Optional indicator LED
#define GPS_RX D7       // GPS TX → NodeMCU D7 (GPIO13)
#define GPS_TX D6       // GPS RX → NodeMCU D6 (GPIO12)

// ==== GPS Setup ====
TinyGPSPlus gps;
SoftwareSerial gpsSerial(GPS_RX, GPS_TX);  // RX, TX

// ==== Telegram Client ====
WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOT_TOKEN, secured_client);

// ==== Default Location (if GPS not ready) ====
float defaultLat = 16.815702;   // Kolhapur
float defaultLon = 74.319992;   // Kolhapur

// ==== Function to Send SOS ====
void sendSOS(float lat, float lon) {
  String msg = "🚨 *SOS ALERT!* 🚨\n";
  msg += "Please help me! I’m in danger!.\n\n";
  msg += "📍 *Location:*\n";
  msg += "Latitude: " + String(lat, 6) + "\n";
  msg += "Longitude: " + String(lon, 6) + "\n";
  msg += "🌍 [Open in Google Maps](https://www.google.com/maps?q=" + String(lat, 6) + "," + String(lon, 6) + ")";
  
  bot.sendMessage(CHAT_ID, msg, "Markdown");
  Serial.println("✅ SOS Message Sent to Telegram!");
}

void setup() {
  Serial.begin(115200);
  gpsSerial.begin(9600);
  
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  // ==== Connect WiFi ====
  Serial.print("Connecting to WiFi: ");
  Serial.println(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("\n✅ WiFi Connected!");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());

  // Disable SSL verification for simplicity
  secured_client.setInsecure();

  Serial.println("✅ Telegram Bot Ready!");
  Serial.println("📡 Waiting for GPS fix...");
}

void loop() {
  // Read GPS continuously
  while (gpsSerial.available() > 0) {
    gps.encode(gpsSerial.read());
  }

  // Check button press (active LOW)
  if (digitalRead(BUTTON_PIN) == LOW) {
    Serial.println("🚨 Button Pressed! Getting GPS...");

    // Try for valid GPS data for 10 seconds
    unsigned long start = millis();
    while (!gps.location.isValid() && millis() - start < 10000) {
      while (gpsSerial.available() > 0) gps.encode(gpsSerial.read());
    }

    float lat = gps.location.isValid() ? gps.location.lat() : defaultLat;
    float lon = gps.location.isValid() ? gps.location.lng() : defaultLon;

    Serial.print("Latitude: "); Serial.println(lat, 6);
    Serial.print("Longitude: "); Serial.println(lon, 6);

    // Blink LED while sending
    digitalWrite(LED_PIN, HIGH);
    sendSOS(lat, lon);
    digitalWrite(LED_PIN, LOW);

    // Prevent multiple sends per press
    delay(3000);
    while (digitalRead(BUTTON_PIN) == LOW); // Wait for release
  }
}
