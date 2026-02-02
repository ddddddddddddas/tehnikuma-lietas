#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>

#include <OneWire.h>
#include <DallasTemperature.h>

const char* ssid = "------";
const char* password = "------";

const char* SERVER_URL = "-----";
const char* API_KEY    = "-----";
const char* DEVICE_ID  = "-----";

#define ONE_WIRE_BUS D2

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

void setup() {
  Serial.begin(9600);
  delay(200);

  WiFi.mode(WIFI_STA);

  // FIX DNS (important!)
  WiFi.config(
    IPAddress(0,0,0,0),
    IPAddress(0,0,0,0),
    IPAddress(0,0,0,0),
    IPAddress(1,1,1,1),
    IPAddress(8,8,8,8)
  );

  Serial.print("Connecting to WiFi");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println("\nWiFi Connected!");
  Serial.println(WiFi.localIP());

  sensors.begin();
}

void loop() {
  sensors.requestTemperatures();
  float tempC = sensors.getTempCByIndex(0);

  Serial.print("Sensor Reading: ");
  Serial.println(tempC);

  if (WiFi.status() == WL_CONNECTED) {

    WiFiClientSecure client;
    client.setInsecure();      // HTTPS fix
    client.setTimeout(15000);

    HTTPClient https;
    https.begin(client, SERVER_URL);

    https.addHeader("Content-Type", "application/x-www-form-urlencoded");

    String postData =
      "api_key=" + String(API_KEY) +
      "&device_id=" + String(DEVICE_ID) +
      "&temp_c=" + String(tempC, 2);

    int httpCode = https.POST(postData);
    String resp  = https.getString();

    Serial.print("HTTP Code: ");
    Serial.println(httpCode);
    Serial.print("Response: ");
    Serial.println(resp);

    https.end();
  }

  delay(2000);
}
