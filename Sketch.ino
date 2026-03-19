#define BLYNK_TEMPLATE_ID ""
#define BLYNK_TEMPLATE_NAME "SentinelX"
#define BLYNK_AUTH_TOKEN "" // <--- PASTE YOUR TOKEN HERE

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h>
#include "DHT.h"

// WiFi Credentials for Wokwi
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
bool isArmed = false;
int16_t last_ax;
const int tamperThreshold = 5000; 

// This function sends data to Blynk every 2 seconds
void myTimerEvent() {
  float temp = dht.readTemperature();
  float hum = dht.readHumidity();
  int gasValue = analogRead(GAS_PIN);

  // V0: Armed Status (1 or 0)
  // V1: Temperature
  // V2: Gas Level
  Blynk.virtualWrite(V0, isArmed ? 1 : 0);
  Blynk.virtualWrite(V1, temp);
  Blynk.virtualWrite(V2, gasValue);
}

void setup() {
  Serial.begin(115200);
  
  // Initialize Blynk & WiFi
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

  // Wake up MPU6050
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x6B);
  Wire.write(0);
  Wire.endTransmission(true);

  // Setup timer to run every 2000ms (2 seconds)
  timer.setInterval(2000L, myTimerEvent);

  Serial.println("--- Aegis Home Guardian Online ---");
}

void loop() {
  Blynk.run();
  timer.run();
  checkRFID();
  
  float temp = dht.readTemperature();
  int gasValue = analogRead(GAS_PIN);
  
  // 1. ALWAYS ACTIVE: Safety Layer
  if (gasValue > 2000 || temp > 50.0) {
    triggerAlarm("SAFETY CRITICAL: GAS OR FIRE!");
    Blynk.logEvent("safety_alert", "Gas or Fire Detected!");
  }

  // 2. CONDITIONAL: Security Layer
  if (isArmed) {
    digitalWrite(LED_PIN, HIGH);
    checkSecuritySensors();
  } else {
    digitalWrite(LED_PIN, LOW);
    noTone(BUZZER_PIN);
  }
}

void checkRFID() {
  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    isArmed = !isArmed; // Toggle State
    Serial.print("System ");
    Serial.println(isArmed ? "ARMED 🔒" : "DISARMED 🔓");
    
    // Send update to Blynk immediately
    Blynk.virtualWrite(V0, isArmed ? 1 : 0);
    Blynk.logEvent("system_update", isArmed ? "System Armed" : "System Disarmed");

    tone(BUZZER_PIN, isArmed ? 1000 : 500, 200);
    delay(500);
    mfrc522.PICC_HaltA();
  }
}

void checkSecuritySensors() {
  // Check Motion
  if (digitalRead(PIR_PIN) == HIGH) {
    triggerAlarm("INTRUSION: Motion!");
    Blynk.logEvent("intrusion", "Motion Detected in Home!");
  }

  // Check Distance
  digitalWrite(TRIG_PIN, LOW); delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH); delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  float distance = pulseIn(ECHO_PIN, HIGH) * 0.034 / 2;
  if (distance < 50 && distance > 1) {
    triggerAlarm("INTRUSION: Proximity!");
    Blynk.logEvent("intrusion", "Someone is at the door!");
  }

  // Check Tamper (MPU6050)
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_ADDR, 6, true);
  int16_t ax = Wire.read()<<8|Wire.read();
  if (abs(ax - last_ax) > tamperThreshold) {
    triggerAlarm("TAMPER: Moved!");
    Blynk.logEvent("tamper", "Security Device was moved!");
  }
  last_ax = ax;
}

void triggerAlarm(String reason) {
  Serial.println("🚨 ALERT: " + reason);
  tone(BUZZER_PIN, 2000); 
  delay(100);
}
