# SentinelX (v1.0) – Project Documentation
============================================

1\. Project Overview
-----------------------

**SentinelX** is an integrated IoT-based security and safety system built using the ESP32 platform.It combines:

*   **Active Security Monitoring** (intrusion/theft detection)
    
*   **Passive Safety Monitoring** (gas leaks, fire risks)
    
*   **Real-time cloud connectivity** via Blynk IoT
    

The system provides continuous monitoring and instant alerts through a mobile dashboard.

2\. Hardware Architecture
----------------------------

The system uses a multi-layered sensor architecture:

### Security Layer

ComponentGPIO PinsFunctionPIR Motion Sensor27Detects human movementUltrasonic Sensor (HC-SR04)12 (Trig), 13 (Echo)Detects proximity/unusual approachRFID Reader (RC522)5, 18, 19, 23, 4Authorized access & system controlMPU6050 (Accel + Gyro)21 (SDA), 22 (SCL)Detects tampering/movement

### Safety Layer

ComponentGPIO PinFunctionGas Sensor (MQ-2)34 (Analog)Detects smoke/gas leaksDHT22 Sensor15Measures temperature & humidity

### Feedback Layer

ComponentGPIO PinFunctionBuzzer32Audible alarmRed LED2Indicates "Armed" status

3\. System Logic & Behavior
------------------------------

### System States

#### **Armed Mode**

*   Activated by scanning an authorized RFID card (BLUE)
*   all other cards are not authorized 
    
*   Security sensors become active:
    
    *   Motion detection (PIR)
        
    *   Proximity detection (Ultrasonic)
    *   humidity temperature detection
    *   gas detection
        

#### **Disarmed Mode**

*   Intrusion alerts are ignored
*   motion of the system is ignored
*   gas and temperature rising is not ignored
    

### Safety Layer (Always Active)

Regardless of system state:

*   Gas level > **2000** → Trigger alarm
    
*   Temperature > **50°C** → Trigger alarm
    

### Smart Behavior

*   RFID acts as a secure toggle between modes
    
*   Sensor data is continuously monitored and processed
    
*   Alerts are triggered only under defined conditions
    

4\. IoT & Cloud Integration
------------------------------

### Connectivity

*   WiFi: **Wokwi-GUEST (Simulation Environment)**
    

### Blynk Datastreams

Virtual PinDescriptionV0Armed / Disarmed statusV1Temperature (°C)V2Gas level

### Notifications

*   Configured using Blynk.logEvent()
    
*   Alerts include:
    
    *   Intrusion detection 
        
    *   Gas leaks 
        
    *   High temperature warnings 



5\. Libraries used
---------------------
1. MFRC522 - Read and write different types of Radio-Frequency IDentification (RFID) cards on your Arduino using a RC522 based reader connected via the Serial Peripheral Interface (SPI) interface.
2. Adafruit MPU6050 - 6-DoF Accelerometer and Gyro Library for Arduino
3. DHT sensor library- An Arduino library for the DHT series of low-cost temperature/humidity sensors.
4. Blynk - It supports WiFi, Ethernet, Cellular connectivity. Works with over 400 boards like ESP8266, ESP32, Arduino, Raspberry Pi, Particle, etc.

6\. Accomplishments
---------------------

*   Successfully integrated SPI, I2C, Analog, and Digital components without conflicts
    
*   Verified RFID-based system state control
    
*   Calibrated ultrasonic sensor (alerts triggered within **50 cm**)
    
*   Achieved successful cloud connection (Blynk status: **Online**)
    
*   Validated real-time sensor data transmission:
    
    *   Temperature: **26.50°C**
        
    *   Gas Level: **1456**
        

Summary
----------

SentinelX demonstrates a complete IoT solution combining:

*   Embedded systems (ESP32)
    
*   Multi-sensor integration
    
*   Real-time cloud communication
    
*   Intelligent security logic
    

This project serves as a strong foundation for advanced systems like:

*   AI-powered surveillance
    
*   Smart home automation
    
*   Industrial safety monitoring
    

**Status:** Functional (Simulation Complete)**Next Version:** 
Hardware Deployment + Advanced Features
