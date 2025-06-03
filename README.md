# Multinode Air Purifier with Recommended Asthmatic Management

This Final Year Project (FYP) aims to develop a smart, scalable, and personalized air purification system designed to help individuals with asthma and other respiratory conditions by monitoring and improving indoor air quality.

## Overview

The system consists of multiple sensor nodes deployed across different indoor zones. These nodes monitor:

- PM2.5
- CO₂
- Temperature
- Humidity

When poor air quality is detected, the system activates air purifiers either **automatically** based on AQI thresholds or **manually** via a **web dashboard**.

## Key Features

- **Real-Time Air Quality Monitoring**
- **Automated & Manual Purifier Control for individuals with asthma**
- **Web Dashboard using ESP32 (HTML/CSS/JS + SPIFFS)**
- **Wireless Multi-node Communication**
- **Energy-efficient & Cost-effective Operation**

## Tech Stack

- **Microcontroller:** ESP32
- **Frontend:** HTML, CSS, JavaScript
- **Backend/Storage:** SPIFFS (ESP32 file system)
- **Sensors Used:** MQ135 sensor, DHT22, DSM501A sensor
- **Communication:** Wi-Fi (HTTP requests between nodes and server)
- **Control:** Relay modules on GPIOs for purifier control

## Dashboard Features

- Real-time display of air quality metrics for each node
- Manual control for asthmatic users (on/off, set duration)
- Historical air quality trends

