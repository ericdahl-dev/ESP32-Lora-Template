#pragma once

#include <stdint.h>

// System-wide configuration management
// Centralized configuration for all modules

namespace SystemConfig {
    // Hardware pin definitions
    namespace Pins {
        constexpr uint8_t VEXT = 36;        // Vext control: LOW = ON
        constexpr uint8_t OLED_RST = 21;    // OLED reset pin
        constexpr uint8_t BUTTON = 0;       // BOOT button on GPIO0 (active LOW)

        // I2C pins for OLED
        constexpr uint8_t I2C_SDA = 17;
        constexpr uint8_t I2C_SCL = 18;

        // LoRa radio pins
        constexpr uint8_t LORA_NSS = 8;
        constexpr uint8_t LORA_DIO1 = 14;
        constexpr uint8_t LORA_RST = 12;
        constexpr uint8_t LORA_BUSY = 13;

        // Future sensor pins
        constexpr uint8_t LIGHTNING_IRQ = 4;    // AS3935 interrupt pin
        constexpr uint8_t LIGHTNING_CS = 5;     // AS3935 chip select

        // Future actuator pins
        constexpr uint8_t LED_DATA = 2;         // WS2812 LED strip data
        constexpr uint8_t BUZZER = 3;           // Buzzer/speaker output
    }

    // LoRa configuration
    namespace LoRa {
        constexpr float DEFAULT_FREQ_MHZ = 915.0f;
        constexpr float DEFAULT_BW_KHZ = 125.0f;
        constexpr uint8_t DEFAULT_SF = 9;
        constexpr uint8_t DEFAULT_CR = 5;
        constexpr int8_t DEFAULT_TX_DBM = 17;

        // Control channel for discovery/sync
        constexpr float CTRL_FREQ_MHZ = DEFAULT_FREQ_MHZ;
        constexpr float CTRL_BW_KHZ = 125.0f;
        constexpr uint8_t CTRL_SF = 9;
        constexpr uint8_t CTRL_CR = 5;

        // Valid ranges
        constexpr uint8_t SF_MIN = 7;
        constexpr uint8_t SF_MAX = 12;
        constexpr float BW_OPTIONS[] = {62.5f, 125.0f, 250.0f, 500.0f};
        constexpr size_t BW_OPTIONS_COUNT = sizeof(BW_OPTIONS) / sizeof(BW_OPTIONS[0]);
    }

    // Button timing configuration
    namespace Button {
        constexpr uint32_t DEBOUNCE_MS = 50;
        constexpr uint32_t SHORT_PRESS_MS = 100;
        constexpr uint32_t MEDIUM_PRESS_MS = 1000;
        constexpr uint32_t LONG_PRESS_MS = 3000;
    }

    // Display configuration
    namespace Display {
        constexpr uint8_t WIDTH = 128;
        constexpr uint8_t HEIGHT = 64;
        constexpr uint32_t UPDATE_INTERVAL_MS = 100;
    }

    // System timing
    namespace Timing {
        constexpr uint32_t MAIN_LOOP_INTERVAL_MS = 10;
        constexpr uint32_t STATUS_UPDATE_INTERVAL_MS = 1000;
        constexpr uint32_t PING_INTERVAL_MS = 2000;
    }

    // WiFi configuration
    namespace WiFi {
        constexpr uint32_t CONNECT_TIMEOUT_MS = 10000;
        constexpr uint32_t RETRY_DELAY_MS = 1000;
        constexpr uint8_t MAX_RETRIES = 3;
    }

    // OTA configuration
    namespace OTA {
        constexpr char HOSTNAME[] = "LtngDet-Receiver";
        constexpr char PASSWORD[] = "123456";
        constexpr size_t FIRMWARE_BUFFER_SIZE = 64 * 1024; // 64KB
        constexpr uint32_t CURRENT_VERSION = 0x010000; // Version 1.0.0
    }

    // Lightning sensor configuration
    namespace Lightning {
        constexpr uint8_t NOISE_FLOOR = 2;
        constexpr uint8_t WATCHDOG_THRESHOLD = 2;
        constexpr uint8_t SPIKE_REJECTION = 2;
        constexpr uint8_t MIN_STRIKES = 5;
    }

    // LED strip configuration
    namespace LEDs {
        constexpr uint16_t COUNT = 16;          // Number of LEDs in ring
        constexpr uint8_t BRIGHTNESS = 128;     // Default brightness (0-255)
        constexpr uint32_t ANIMATION_SPEED_MS = 50;
    }
}
