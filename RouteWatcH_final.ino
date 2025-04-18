#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <time.h>
#include <TinyGPSPlus.h>
#include <HardwareSerial.h>

// WiFi Credentials
#define WIFI_SSID "BALU"
#define WIFI_PASSWORD "04222422980"

// Google API Key
#define GOOGLE_API_KEY "AIzaSyCNAYR0O7rPzS9g_CQADiHV2iCO7qgu3Ls"

// NTP Configuration (GMT +5:30)
const long gmtOffset_sec = 19800;
const int daylightOffset_sec = 0;
const char* ntpServer = "132.163.96.1";

// Timing
const unsigned long POST_INTERVAL = 30000;
unsigned long lastPostTime = 0;

// GPS setup
TinyGPSPlus gps;
HardwareSerial gpsSerial(2); // Use UART2 on ESP32 (GPIO16-RX, GPIO17-TX)

void setup() {
  Serial.begin(115200);
  gpsSerial.begin(9600, SERIAL_8N1, 16, 17);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n✅ Wi-Fi Connected!");

  // NTP Time Sync
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  struct tm timeinfo;
  int retry = 0;
  const int maxRetries = 10;
  while (!getLocalTime(&timeinfo) && retry < maxRetries) {
    Serial.println("⏳ Waiting for NTP time sync...");
    delay(1000);
    retry++;
  }

  if (retry >= maxRetries) {
    Serial.println("❌ NTP sync failed! Proceeding without time...");
  } else {
    Serial.println("✅ NTP time synced.");
  }
}

void loop() {
  // Feed GPS data
  while (gpsSerial.available() > 0) {
    gps.encode(gpsSerial.read());
  }

  unsigned long currentMillis = millis();

  if (currentMillis - lastPostTime >= POST_INTERVAL) {
    lastPostTime = currentMillis;

    if (gps.location.isValid()) {
      float lat = gps.location.lat();
      float lon = gps.location.lng();
      Serial.printf("📡 GPS Location: %.6f, %.6f\n", lat, lon);
      getLocationDetails(lat, lon);
    } else {
      Serial.println("⚠️ GPS location not valid yet. Make sure GPS has fix.");
    }
  }
}

void getLocationDetails(float lat, float lon) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("❌ WiFi disconnected!");
    return;
  }

  HTTPClient http;
  String url = "https://maps.googleapis.com/maps/api/geocode/json?latlng=" +
               String(lat, 6) + "," + String(lon, 6) +
               "&key=" + GOOGLE_API_KEY;

  http.begin(url);
  int httpCode = http.GET();

  if (httpCode == 200) {
    String payload = http.getString();
    DynamicJsonDocument doc(15000);

    DeserializationError error = deserializeJson(doc, payload);
    if (error) {
      Serial.println("❌ JSON Parsing Error!");
      Serial.println(error.f_str());
      http.end();
      return;
    }

    String locality = "", city = "", state = "";
    JsonArray results = doc["results"];
    if (results.size() > 0) {
      JsonArray components = results[0]["address_components"];

      for (JsonObject comp : components) {
        JsonArray types = comp["types"];
        String name = comp["long_name"];

        for (String type : types) {
          if (type == "sublocality_level_1") locality = name;
          else if (type == "locality") city = name;
          else if (type == "administrative_area_level_1") state = name;
        }
      }

      struct tm timeinfo;
      char timestamp[30] = "Unavailable";
      if (getLocalTime(&timeinfo)) {
        strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", &timeinfo);
      }

      Serial.println("⏱ Timestamp: " + String(timestamp));
      Serial.println("📍 Locality: " + locality);
      Serial.println("🏙 City: " + city);
      Serial.println("🌍 State: " + state);

      HTTPClient postHttp;
      postHttp.begin("http://192.168.0.106:5000/addLocation");
      postHttp.addHeader("Content-Type", "application/json");

      String jsonData = "{\"locality\":\"" + locality +
                        "\",\"city\":\"" + city +
                        "\",\"state\":\"" + state +
                        "\",\"timestamp\":\"" + String(timestamp) + "\"}";

      int postCode = postHttp.POST(jsonData);
      if (postCode > 0) {
        Serial.println("✅ Sent to server.");
        Serial.println(postHttp.getString());
      } else {
        Serial.printf("❌ POST Failed! Code: %d\n", postCode);
      }
      postHttp.end();

    } else {
      Serial.println("❌ No address results found.");
    }

  } else {
    Serial.printf("❌ HTTP Request failed! Code: %d\n", httpCode);
  }

  http.end();
}
