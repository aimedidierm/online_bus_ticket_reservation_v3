#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <ArduinoJson.h>


#define RST_PIN         D3
#define SS_PIN          D4
MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance.
LiquidCrystal_I2C lcd(0x27, 20, 4);

const char* ssid = "Balance";
const char* password = "balance1234";
int buzzerPin = D0;
int ledPin = D8;
String content = "";


void setup()
{
  Serial.begin(9600);
  SPI.begin();
  pinMode(buzzerPin, OUTPUT);
  pinMode(ledPin, OUTPUT);
  mfrc522.PCD_Init();
  lcd.init();
  lcd.init();
  lcd.clear();
  lcd.backlight();
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  lcd.setCursor(0, 0);
  lcd.print("Connecting to");
  lcd.setCursor(0, 1);
  lcd.print("WiFi network");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
  }
  lcd.clear(),
            lcd.print("System");
  lcd.setCursor(0, 1);
  lcd.print("Starting");
  delay(1000);
  lcd.clear(),
            lcd.print("System");
  lcd.setCursor(0, 1);
  lcd.print("Ready");
  delay(2000);
  lcd.clear(),
            lcd.print("System");
  lcd.setCursor(0, 1);
  lcd.print("Ready");
  delay(3000);
  lcd.clear(),
            lcd.print("Kozaho ikarita");
}
void loop()
{
  readcard();
}

void readcard() {
  if (!mfrc522.PICC_IsNewCardPresent()) {
    return;
  }
  if (!mfrc522.PICC_ReadCardSerial()) {
    return;
  }
  Serial.print("UID tag: ");
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    content += (mfrc522.uid.uidByte[i] < 0x10 ? "0" : "");
    content += String(mfrc522.uid.uidByte[i], HEX);
  }
  lcd.clear(),
            lcd.print("Tegereza");
  lcd.setCursor(0, 1);
  lcd.print("Kureba amakuru");
  updateStatus();
}

void updateStatus() {
  if (WiFi.status() == WL_CONNECTED) {

    std::unique_ptr<BearSSL::WiFiClientSecure> client(new BearSSL::WiFiClientSecure);

    // Ignore SSL certificate validation (for testing only)
    client->setInsecure();
    HTTPClient http;

    Serial.println("[HTTPS] begin...");

    if (http.begin(*client, "https://flugence.plantymart.xyz/api/status")) {  // HTTPS
      Serial.println(content);
      Serial.println("[HTTPS] POST...");

      http.addHeader("Content-Type", "application/json");

      DynamicJsonDocument jsonDoc(128); // Adjust the buffer size as needed

      jsonDoc["card"] = content;

      String jsonPayload;
      serializeJson(jsonDoc, jsonPayload);

      // Send the POST request with the JSON payload
      int httpCode = http.POST(jsonPayload);

      // Check the HTTP response code
      if (httpCode > 0) {
        content = "";
        Serial.printf("[HTTPS] POST... code: %d\n", httpCode);

        // Read and print the response from the server
        String payload = http.getString();
        Serial.println(payload);
        DynamicJsonDocument doc(512); // Adjust the size based on your payload size
        DeserializationError error = deserializeJson(doc, payload);

        // Check for parsing errors
        if (error) {
          Serial.print(F("Error parsing JSON: "));
          Serial.println(error.c_str());
        } else {
          // Access JSON data
          bool cardAllowed = doc["card_allowed"];
          String message = doc["message"];

          if (cardAllowed == true)
          {
            lcd.clear(),
                      lcd.print("Uremerewe!");
            lcd.setCursor(0, 1);
            lcd.print(message);
            playBeep(1000, 500);
            delay(3000);
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Kozaho ikarita");
            readcard();
          }
          else {
            lcd.clear(),
                      lcd.print("Ikibazo");
            lcd.setCursor(0, 1);
            lcd.print(message);
            playAlarm();
            delay(3000);
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Kozaho ikarita");
            readcard();
          }
        }
      } else {
        content = "";
        Serial.printf("[HTTPS] POST... failed, error: %s\n", http.errorToString(httpCode).c_str());
        lcd.clear(),
                  lcd.print("Ntibikunze");
                  playAlarm();
        delay(3000);
        readcard();
      }

      // End the HTTP connection
      http.end();
    } else {
      Serial.println("[HTTPS] Unable to connect");
      lcd.clear(),
                lcd.print("Guhuza ntibikunze");
                playAlarm();
      delay(3000);
      readcard();
    }
  }
}

void playBeep(int frequency, int duration) {
  tone(buzzerPin, frequency, duration);
  delay(duration);
  noTone(buzzerPin);
}

void playAlarm() {
  for (int i = 0; i < 3; i++) {
    digitalWrite(ledPin, HIGH);
    playBeep(1000, 200);
    digitalWrite(ledPin, LOW);
    delay(200);
  }
}
