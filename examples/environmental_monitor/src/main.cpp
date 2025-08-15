/**
 * Environmental Monitor Example
 * 
 * This example shows how to use the ESP32 template for environmental monitoring.
 * It demonstrates sensor integration, display output, and power management.
 */

#include <Arduino.h>
#include "hardware/hardware_abstraction.h"
#include "sensors/dht_sensor.h"
#include "sensors/light_sensor.h"
#include "actuators/oled_display.h"
#include "system/power_manager.h"
#include "wifi_manager.h"
#include <SD.h>

// Sensor instances
DHTSensor dhtSensor(DHT_PIN, DHT_TYPE);
LightSensor lightSensor;
OLEDDisplay display;

// Data storage
struct SensorData {
    float temperature;
    float humidity;
    int airQuality;
    float lightLevel;
    float batteryVoltage;
    unsigned long timestamp;
} currentData;

// Function prototypes
void readSensors();
void displayData();
void logDataToSD();
void connectToWiFi();

void setup() {
    Serial.begin(115200);
    Serial.println("Environmental Monitor Starting...");
    
    // Initialize HAL
    HAL::init();
    
    // Initialize display
    display.begin();
    display.showMessage("Initializing...");
    
    // Initialize sensors
    dhtSensor.begin();
    lightSensor.begin();
    
    // Initialize SD card if enabled
    #ifdef ENABLE_SD_CARD
    if (!SD.begin(SD_CS_PIN)) {
        Serial.println("SD Card initialization failed!");
    } else {
        Serial.println("SD Card initialized.");
    }
    #endif
    
    // Connect to WiFi
    connectToWiFi();
    
    // Setup complete
    display.showMessage("Ready!");
    delay(2000);
}

void loop() {
    // Read all sensors
    readSensors();
    
    // Update display
    displayData();
    
    // Log to SD card
    #ifdef ENABLE_SD_CARD
    logDataToSD();
    #endif
    
    // Print to serial for debugging
    Serial.printf("Temp: %.1f°C, Humidity: %.1f%%, Light: %.1f lux\n", 
                  currentData.temperature, currentData.humidity, currentData.lightLevel);
    
    // Deep sleep if enabled
    #ifdef ENABLE_DEEP_SLEEP
    Serial.println("Going to sleep...");
    display.clear();
    HAL::Power::deepSleep(SLEEP_INTERVAL);
    #else
    delay(SLEEP_INTERVAL);
    #endif
}

void readSensors() {
    currentData.timestamp = millis();
    
    // Read DHT sensor
    currentData.temperature = dhtSensor.readTemperature();
    currentData.humidity = dhtSensor.readHumidity();
    
    // Read air quality
    #ifdef ENABLE_AIR_QUALITY
    currentData.airQuality = analogRead(AIR_QUALITY_PIN);
    #endif
    
    // Read light sensor
    currentData.lightLevel = lightSensor.readLux();
    
    // Read battery voltage
    #ifdef ENABLE_BATTERY_MONITOR
    currentData.batteryVoltage = HAL::Power::getBatteryVoltage();
    #endif
}

void displayData() {
    display.clear();
    
    // Line 1: Temperature and Humidity
    display.setCursor(0, 0);
    display.printf("T:%.1fC H:%.1f%%", currentData.temperature, currentData.humidity);
    
    // Line 2: Light level
    display.setCursor(0, 16);
    display.printf("Light: %.1f lux", currentData.lightLevel);
    
    // Line 3: Air quality
    #ifdef ENABLE_AIR_QUALITY
    display.setCursor(0, 32);
    display.printf("Air: %d", currentData.airQuality);
    #endif
    
    // Line 4: Battery
    #ifdef ENABLE_BATTERY_MONITOR
    display.setCursor(0, 48);
    display.printf("Bat: %.2fV", currentData.batteryVoltage);
    #endif
    
    display.update();
}

void logDataToSD() {
    #ifdef ENABLE_SD_CARD
    File dataFile = SD.open("/envdata.csv", FILE_APPEND);
    
    if (dataFile) {
        // Write CSV header if file is new
        if (dataFile.size() == 0) {
            dataFile.println("timestamp,temperature,humidity,air_quality,light_level,battery_voltage");
        }
        
        // Write data
        dataFile.printf("%lu,%.2f,%.2f,%d,%.2f,%.2f\n",
                       currentData.timestamp,
                       currentData.temperature,
                       currentData.humidity,
                       currentData.airQuality,
                       currentData.lightLevel,
                       currentData.batteryVoltage);
        
        dataFile.close();
        Serial.println("Data logged to SD card");
    } else {
        Serial.println("Error opening data file");
    }
    #endif
}

void connectToWiFi() {
    #ifdef ENABLE_WIFI
    WiFiManager::begin();
    
    if (WiFiManager::connect()) {
        Serial.println("WiFi connected!");
        display.showMessage("WiFi OK");
        
        // Enable OTA if configured
        #ifdef ENABLE_OTA
        WiFiManager::enableOTA("EnvMonitor");
        #endif
    } else {
        Serial.println("WiFi connection failed");
        display.showMessage("No WiFi");
    }
    #endif
}