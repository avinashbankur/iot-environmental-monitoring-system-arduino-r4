IoT Environmental Monitoring System

An IoT-based embedded system that monitors temperature, humidity, and air quality in real-time using Arduino UNO R4 WiFi and publishes data via MQTT.

Features

Temperature & Humidity Monitoring (DHT11)
Air Quality Detection (MQ135)
Real-time LCD Display (16x2 I2C)
MQTT-based Remote Monitoring
Remote LED Control via MQTT
WiFi-enabled IoT Communication

Hardware Components

Arduino UNO R4 WiFi used as Main microcontroller
DHT11 Temperature & humidity sensor
MQ135 Air quality sensor
LCD 16x2 (I2C) Display module
LED Output indicator

System Architecture

Sensors collect environmental data
Arduino processes and displays on LCD
Data is sent to MQTT broker
Remote commands control LED

MQTT Topics:

Publish: `iot/env_monitor`
Subscribe: `iot/env_monitor/cmd`
