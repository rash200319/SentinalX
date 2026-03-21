#define BLYNK_TEMPLATE_ID "TMPL66EHBbSgs"
#define BLYNK_TEMPLATE_NAME "SentinelX"
#define BLYNK_AUTH_TOKEN "OeGjJkKpywz7WwNPKdPyEStyt8Y9UprG"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h>
#include "DHT.h"

char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "Wokwi-GUEST";
char pass[] = "";

// Pin Definitions
#define RST_PIN         4          
#define SS_PIN          5          
#define TRIG_PIN        12         
#define ECHO_PIN        13         
#define PIR_PIN         27         
#define DHTPIN          15         
#define GAS_PIN         34         
#define BUZZER_PIN      32         
#define LED_PIN         2          
#define MPU_ADDR        0x68

#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);
MFRC522 mfrc522(SS_PIN, RST_PIN);
BlynkTimer timer;

// System Variables
bool isArmed = true; // Armed at start as requested
int16_t last_ax = 0;
const int tamperThreshold = 8000; // Increased threshold to avoid sensitive buzzing
unsigned long lastAlarmTime = 0;
const int alarmInterval = 2000; // Only print/buzz every 2 seconds to avoid "nonstop" noise

void myTimerEvent() {
  float temp = dht.readTemperature();
  int gasValue = analogRead(GAS_PIN);
  Blynk.virtualWrite(V0, isArmed ? 1 : 0);
  Blynk.virtualWrite(V1, temp);
  Blynk.virtualWrite(V2, gasValue);
}

void setup() {
  Serial.begin(115200);
  Blynk.begin(auth, ssid, pass);

  SPI.begin();
  mfrc522.PCD_Init();
  dht.begin();
  Wire.begin(21, 22);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(PIR_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);

  // MPU6050 Wakeup
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x6B);
  Wire.write(0);
  Wire.endTransmission(true);

  // Initial MPU reading to prevent immediate tamper alarm
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_ADDR, 2, true);
  last_ax = Wire.read()<<8|Wire.read();

  timer.setInterval(2000L, myTimerEvent);
  Serial.println("--- SentinelX System: ARMED & ONLINE ---");
}

void loop() {
  Blynk.run();
  timer.run();
  checkRFID();
  
  float temp = dht.readTemperature();
  int gasValue = analogRead(GAS_PIN);
  
  // 1. SAFETY LAYER: Always Active
  if (gasValue > 2500 || temp > 55.0) {
    triggerAlarm("CRITICAL: GAS/FIRE LEAK!");
  }

  // 2. SECURITY LAYER: Only if Armed
  if (isArmed) {
    digitalWrite(LED_PIN, HIGH);
    checkSecuritySensors();
  } else {
    // DISARMED: Turn everything off
    digitalWrite(LED_PIN, LOW);
    noTone(BUZZER_PIN); 
  }
}

void checkRFID() {
  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    byte blueCard[4] = {0x01, 0x02, 0x03, 0x04}; // Ensure this matches your blue card UID
    bool isAuthorized = true;
    for (byte i = 0; i < 4; i++) {
      if (mfrc522.uid.uidByte[i] != blueCard[i]) isAuthorized = false;
    }

    if (isAuthorized) {
      isArmed = !isArmed;
      Serial.print("User Verified. System: ");
      Serial.println(isArmed ? "ARMED 🔒" : "DISARMED (Standby Mode) 🔓");
      
      if (!isArmed) noTone(BUZZER_PIN); // Kill alarm immediately on disarm
      
      Blynk.virtualWrite(V0, isArmed ? 1 : 0);
      tone(BUZZER_PIN, isArmed ? 1000 : 500, 300);
    } else {
      Serial.println("Unauthorized Key 🚫");
      triggerAlarm("TAMPER: Invalid RFID Attempt!");
    }
    delay(500);
    mfrc522.PICC_HaltA();
  }
}

void checkSecuritySensors() {
  // Motion
  if (digitalRead(PIR_PIN) == HIGH) {
    triggerAlarm("INTRUSION: Motion Detected");
  }

  // Proximity
  digitalWrite(TRIG_PIN, LOW); delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH); delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  float distance = pulseIn(ECHO_PIN, HIGH) * 0.034 / 2;
  if (distance < 50 && distance > 1) {
    triggerAlarm("INTRUSION: Proximity Alert");
  }

  // Tamper
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_ADDR, 2, true);
  int16_t ax = Wire.read()<<8|Wire.read();
  if (abs(ax - last_ax) > tamperThreshold) {
    triggerAlarm("TAMPER: System Movement");
  }
  last_ax = ax;
}

void triggerAlarm(String reason) {
  // Only trigger if at least 2 seconds have passed since the last pulse
  if (millis() - lastAlarmTime > alarmInterval) {
    Serial.println("🚨 " + reason);
    Blynk.logEvent("intrusion", reason); // Sends alert to phone
    tone(BUZZER_PIN, 2000, 1000); // Buzz for 1 second
    lastAlarmTime = millis();
  }
}
