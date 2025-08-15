#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>
#include <SPI.h>
#include <RadioLib.h>
#include <Preferences.h>

#ifdef ENABLE_WIFI_OTA
#include <WiFi.h>
#include <ArduinoOTA.h>
#include <Update.h>
#endif
// Allow sender to flash firmware received over LoRa
#if defined(ROLE_SENDER) && !defined(ENABLE_WIFI_OTA)
#include <Update.h>
#endif

// Vext power control and OLED reset (Heltec V3)
#define VEXT_PIN 36        // Vext control: LOW = ON
#define OLED_RST_PIN 21    // OLED reset pin
#define BUTTON_PIN 0       // BOOT button on GPIO0 (active LOW)

// SSD1306 128x64 OLED over HW I2C; pins set via Wire.begin(17,18)
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

#ifndef PIN_LORA_NSS
  #define PIN_LORA_NSS   8
#endif
#ifndef PIN_LORA_DIO1
  #define PIN_LORA_DIO1  14
#endif
#ifndef PIN_LORA_RST
  #define PIN_LORA_RST   12
#endif
#ifndef PIN_LORA_BUSY
  #define PIN_LORA_BUSY  13
#endif

#ifndef LORA_FREQ_MHZ
  #define LORA_FREQ_MHZ  915.0
#endif
#ifndef LORA_BW_KHZ
  #define LORA_BW_KHZ    125.0
#endif
#ifndef LORA_SF
  #define LORA_SF        9
#endif
#ifndef LORA_CR
  #define LORA_CR        5
#endif
#ifndef LORA_TX_DBM
  #define LORA_TX_DBM    17
#endif

// Control channel used for discovery/sync at boot
#ifndef CTRL_FREQ_MHZ
  #define CTRL_FREQ_MHZ  LORA_FREQ_MHZ
#endif
#ifndef CTRL_BW_KHZ
  #define CTRL_BW_KHZ    125.0
#endif
#ifndef CTRL_SF
  #define CTRL_SF        9
#endif
#ifndef CTRL_CR
  #define CTRL_CR        5
#endif

// WiFi and OTA Configuration (Receiver only)
#ifdef ENABLE_WIFI_OTA
#include "wifi_manager.h"

// Firmware storage for LoRa OTA cascade updates
static uint8_t storedFirmware[64 * 1024]; // 64KB buffer for firmware storage (reduced for DRAM)
static size_t storedFirmwareSize = 0;
static bool hasStoredFirmware = false;
static uint32_t firmwareVersion = 0x010000; // Version 1.0.0
#endif

SX1262 radio = new Module(PIN_LORA_NSS, PIN_LORA_DIO1, PIN_LORA_RST, PIN_LORA_BUSY);
static Preferences prefs;

static bool isSender = true;
static uint32_t seq = 0;
static uint32_t lastButtonMs = 0;
static int lastButtonState = HIGH;
static uint32_t buttonPressMs = 0;
static bool buttonPressed = false;

// LoRa parameters that can be changed at runtime
static float currentFreq = LORA_FREQ_MHZ;
static float currentBW = LORA_BW_KHZ;
static int currentSF = LORA_SF;
static int currentCR = LORA_CR;
static int currentTxPower = LORA_TX_DBM;

// LoRa parameter arrays for cycling through values
static const int sfValues[] = {7, 8, 9, 10, 11, 12};
static const float bwValues[] = {62.5f, 125.0f, 250.0f, 500.0f};
static const int txPowerValues[] = {2, 3, 5, 8, 10, 12, 15, 17, 20, 22};

// Current indices for parameter cycling
static size_t currentSfIndex = 2;  // Default to SF9
static size_t currentBwIndex = 1;  // Default to 125kHz
static size_t currentTxIndex = 7;  // Default to 17dBm

// Signal quality tracking
static float lastRSSI = -999.0;
static float lastSNR = -999.0;
static uint32_t lastPacketTime = 0;
static uint32_t packetCount = 0;
static uint32_t errorCount = 0;

// Available values for cycling
// Old arrays removed - using new arrays defined above

// Config broadcast state (sender)
static bool pendingConfigBroadcast = false;
static float pendingFreq = 0;
static float pendingBW = 0;
static int pendingSF = 0;
static int pendingCR = 0;
static int pendingTxPower = 0;
static uint32_t cfgLastTxMs = 0;
static int cfgRemaining = 0;

// OTA Update state
#ifdef ENABLE_WIFI_OTA
static bool wifiConnected = false;
static bool otaActive = false;
static uint32_t lastOtaCheck = 0;
#endif

// LoRa OTA state (both sender and receiver)
static bool loraOtaActive = false;
static uint32_t loraOtaStartTime = 0;
static uint32_t loraOtaTimeout = 30000; // 30 seconds timeout
static uint8_t loraOtaBuffer[1024]; // Buffer for OTA data
static size_t loraOtaBufferSize = 0;
static uint32_t loraOtaExpectedSize = 0;
static uint32_t loraOtaReceivedSize = 0;

// Persistence helpers
static void savePersistedSettings();
static void savePersistedRole();
static void loadPersistedSettingsAndRole();
static void computeIndicesFromCurrent();
static void broadcastConfigOnControlChannel(uint8_t times = 8, uint32_t intervalMs = 300);
static void tryReceiveConfigOnControlChannel(uint32_t durationMs = 4000);

// Draw status bar at the bottom of the screen
static void drawStatusBar() {
  u8g2.setFont(u8g2_font_5x7_tr); // Smaller font for status bar

  const int yPos = 120; // Bottom of screen
  int xPos = 2;

#ifdef ENABLE_WIFI_OTA
  if (!isSender) {
    // WiFi status
    if (wifiConnected) {
      const char* location = getCurrentNetworkLocation();
      u8g2.drawStr(xPos, yPos, location);
      xPos += (strlen(location) * 6); // Approximate character width
    } else {
      u8g2.drawStr(xPos, yPos, "NoWiFi");
      xPos += 20;
    }

    // OTA status
    if (otaActive) {
      u8g2.drawStr(xPos, yPos, "OTA");
      xPos += 20;
    }

    // LoRa OTA status
    if (loraOtaActive) {
      u8g2.drawStr(xPos, yPos, "LoRaOTA");
    }
  }
#endif
}

static void oledMsg(const char* l1, const char* l2 = nullptr, const char* l3 = nullptr) {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x10_tr);

  // Portrait layout: 64 pixels wide, 128 pixels tall

  // Top section - main status
  if (l1) u8g2.drawStr(2, 12, l1);

  // Second line - additional info (if provided) - moved down one line
  if (l2) u8g2.drawStr(2, 32, l2);

  // Middle section - signal quality for receiver mode
  if (!isSender && lastRSSI > -999.0) {
    char rssiStr[12], snrStr[12];
    snprintf(rssiStr, sizeof(rssiStr), "RSSI: %.0f", lastRSSI);
    snprintf(snrStr, sizeof(snrStr), "SNR: %.1f", lastSNR);

    u8g2.drawStr(2, 51, rssiStr);
    u8g2.drawStr(2, 65, snrStr);
  }

  // Bottom section - settings (moved up to make room for status bar)
  char settings[32];
  snprintf(settings, sizeof(settings), "SF%d BW%.0f", currentSF, currentBW);
  u8g2.drawStr(2, 81, settings);

  char modeStr[16];
  snprintf(modeStr, sizeof(modeStr), "%s %.1fMHz", isSender ? "TX" : "RX", currentFreq);
  u8g2.drawStr(2, 95, modeStr);

  // Status bar at the bottom - WiFi and OTA status
  drawStatusBar();

  u8g2.sendBuffer();
}

static void oledRole() {
  oledMsg("Mode", isSender ? "Sender" : "Receiver");
}

static void oledSettings() {
  oledMsg("Settings", "Updated");
}

static void startConfigBroadcast(float newFreq, float newBW, int newSF, int newCR, int newTxPower) {
  pendingConfigBroadcast = true;
  pendingFreq = newFreq;
  pendingBW = newBW;
  pendingSF = newSF;
  pendingCR = newCR;
  pendingTxPower = newTxPower;
  cfgLastTxMs = 0;
  cfgRemaining = 8; // send several times for reliability
  oledMsg("Syncing...", "Sending config");
}

static void computeIndicesFromCurrent() {
      for (size_t i = 0; i < (sizeof(sfValues) / sizeof(sfValues[0])); i++) {
      if (sfValues[i] == currentSF) { currentSfIndex = i; break; }
    }
    for (size_t i = 0; i < (sizeof(bwValues) / sizeof(bwValues[0])); i++) {
      if (bwValues[i] == currentBW) { currentBwIndex = i; break; }
    }
    for (size_t i = 0; i < (sizeof(txPowerValues) / sizeof(txPowerValues[0])); i++) {
      if (txPowerValues[i] == currentTxPower) { currentTxIndex = i; break; }
    }
}

static void savePersistedSettings() {
  prefs.begin("LtngDet", false);
  prefs.putFloat("freq", currentFreq);
  prefs.putFloat("bw", currentBW);
  prefs.putInt("sf", currentSF);
  prefs.putInt("cr", currentCR);
  prefs.putInt("tx", currentTxPower);
  prefs.end();
}

static void savePersistedRole() {
  prefs.begin("LtngDet", false);
  prefs.putBool("sender", isSender);
  prefs.end();
}

static void loadPersistedSettingsAndRole() {
  prefs.begin("LtngDet", true);
  bool haveFreq = prefs.isKey("freq");
  bool haveBW = prefs.isKey("bw");
  bool haveSF = prefs.isKey("sf");
  bool haveCR = prefs.isKey("cr");
  bool haveTX = prefs.isKey("tx");
  bool haveRole = prefs.isKey("sender");

  if (haveFreq) currentFreq = prefs.getFloat("freq", currentFreq);
  if (haveBW) currentBW = prefs.getFloat("bw", currentBW);
  if (haveSF) currentSF = prefs.getInt("sf", currentSF);
  if (haveCR) currentCR = prefs.getInt("cr", currentCR);
  if (haveTX) currentTxPower = prefs.getInt("tx", currentTxPower);
  if (haveRole) isSender = prefs.getBool("sender", isSender);
  prefs.end();
}

static void initDisplay() {
  // Power OLED via Vext and reset it
  pinMode(VEXT_PIN, OUTPUT);
  digitalWrite(VEXT_PIN, LOW);   // enable Vext
  delay(50);

  pinMode(OLED_RST_PIN, OUTPUT);
  digitalWrite(OLED_RST_PIN, LOW);
  delay(20);
  digitalWrite(OLED_RST_PIN, HIGH);
  delay(50);

  // I2C
  Wire.begin(17, 18);
  Wire.setTimeOut(1000);
  Wire.setClock(100000);
  delay(100);

  u8g2.setI2CAddress(0x3C << 1);
  if (!u8g2.begin()) {
    u8g2.setI2CAddress(0x3D << 1);
    if (!u8g2.begin()) {
      // give up
      while (true) {
        Serial.println("OLED init failed");
        delay(1000);
      }
    }
  }
  u8g2.setPowerSave(0);
  u8g2.setContrast(255);

  // Rotate display 90 degrees for portrait orientation
  u8g2.setDisplayRotation(U8G2_R1);
}

static void updateRadioSettings() {
  int st = radio.setFrequency(currentFreq);
  if (st == RADIOLIB_ERR_NONE) {
    st = radio.setBandwidth(currentBW);
  }
  if (st == RADIOLIB_ERR_NONE) {
    st = radio.setSpreadingFactor(currentSF);
  }
  if (st == RADIOLIB_ERR_NONE) {
    st = radio.setCodingRate(currentCR);
  }
  if (st == RADIOLIB_ERR_NONE) {
    st = radio.setOutputPower(currentTxPower);
  }

  if (st != RADIOLIB_ERR_NONE) {
    Serial.printf("Failed to update radio settings: %d\n", st);
    char errBuf[16]; snprintf(errBuf, sizeof(errBuf), "Settings fail %d", st);
    oledMsg("Settings fail", errBuf);
  } else {
    Serial.printf("Radio updated: SF%d BW%.0f Tx%ddBm\n", currentSF, currentBW, currentTxPower);
    oledSettings();
  }
}

static void initRadioOrHalt() {
  Serial.println("Initializing LoRa radio...");
  int st = radio.begin(currentFreq, currentBW, currentSF, currentCR, 0x34, currentTxPower);
  if (st != RADIOLIB_ERR_NONE) {
    char buf[48]; snprintf(buf, sizeof(buf), "Radio fail %d", st);
    oledMsg("Radio init", buf);
    while (true) { Serial.println(buf); delay(1000); }
  }
  radio.setDio2AsRfSwitch(true);
  radio.setCRC(true);
  oledSettings();
}

static void broadcastConfigOnControlChannel(uint8_t times, uint32_t intervalMs) {
  // Switch to control channel
  int st = radio.begin(CTRL_FREQ_MHZ, CTRL_BW_KHZ, CTRL_SF, CTRL_CR, 0x34, currentTxPower);
  if (st != RADIOLIB_ERR_NONE) {
    Serial.printf("[CTRL] begin fail %d\n", st);
    return;
  }
  radio.setDio2AsRfSwitch(true);
  radio.setCRC(true);

  char msg[64];
  snprintf(msg, sizeof(msg), "CFG F=%.1f BW=%.0f SF=%d CR=%d TX=%d",
           currentFreq, currentBW, currentSF, currentCR, currentTxPower);

  for (uint8_t i = 0; i < times; i++) {
    int tx = radio.transmit(msg);
    Serial.printf("[CTRL][TX] %s %s\n", msg, tx == RADIOLIB_ERR_NONE ? "OK" : "FAIL");
    delay(intervalMs);
  }

  // Restore operational settings
  st = radio.begin(currentFreq, currentBW, currentSF, currentCR, 0x34, currentTxPower);
  if (st != RADIOLIB_ERR_NONE) {
    Serial.printf("[CTRL] restore begin fail %d\n", st);
  } else {
    radio.setDio2AsRfSwitch(true);
    radio.setCRC(true);
  }
}

static void tryReceiveConfigOnControlChannel(uint32_t durationMs) {
  // Switch to control channel
  int st = radio.begin(CTRL_FREQ_MHZ, CTRL_BW_KHZ, CTRL_SF, CTRL_CR, 0x34, currentTxPower);
  if (st != RADIOLIB_ERR_NONE) {
    Serial.printf("[CTRL] begin fail %d\n", st);
    return;
  }
  radio.setDio2AsRfSwitch(true);
  radio.setCRC(true);

  uint32_t start = millis();
  while (millis() - start < durationMs) {
    String rx;
    int r = radio.receive(rx);
    if (r == RADIOLIB_ERR_NONE && rx.startsWith("CFG ")) {
      float nf = currentFreq;
      float nb = currentBW;
      int nsf = currentSF;
      int ncr = currentCR;
      int ntx = currentTxPower;
      int parsed = sscanf(rx.c_str(), "CFG F=%f BW=%f SF=%d CR=%d TX=%d", &nf, &nb, &nsf, &ncr, &ntx);
      if (parsed == 5) {
        currentFreq = nf;
        currentBW = nb;
        currentSF = nsf;
        currentCR = ncr;
        currentTxPower = ntx;
        computeIndicesFromCurrent();
        savePersistedSettings();
        Serial.printf("[CTRL][RX] applied %s\n", rx.c_str());
        break;
      }
    }
    delay(50);
  }

  // Restore operational settings (applied ones if updated)
  st = radio.begin(currentFreq, currentBW, currentSF, currentCR, 0x34, currentTxPower);
  if (st != RADIOLIB_ERR_NONE) {
    Serial.printf("[CTRL] restore begin fail %d\n", st);
  } else {
    radio.setDio2AsRfSwitch(true);
    radio.setCRC(true);
  }
}

static void updateButton() {
  int s = digitalRead(BUTTON_PIN);
  uint32_t now = millis();

  // Button press detection
  if (lastButtonState == HIGH && s == LOW) {
    buttonPressed = true;
    buttonPressMs = now;
  }

  // Button release detection
  if (lastButtonState == LOW && s == HIGH && buttonPressed) {
    buttonPressed = false;
    uint32_t pressDuration = now - buttonPressMs;

    if (pressDuration < 100) {
      // Very short press - ignore (debounce)
    } else if (pressDuration < 1000) {
      // Short press - toggle mode
      isSender = !isSender;
      seq = 0;
      savePersistedRole();
      oledRole();
      Serial.printf("Switched mode -> %s\n", isSender ? "Sender" : "Receiver");
    } else if (pressDuration < 3000) {
      // Medium press - cycle SF (sender) or network mode (receiver)
      if (isSender) {
            const int nextIndex = (currentSfIndex + 1) % (sizeof(sfValues) / sizeof(sfValues[0]));
    const int nextSF = sfValues[nextIndex];
        startConfigBroadcast(currentFreq, currentBW, nextSF, currentCR, currentTxPower);
        Serial.printf("SF change requested -> %d (broadcasting to receiver)\n", nextSF);
      } else {
#ifdef ENABLE_WIFI_OTA
        // Cycle through network modes for receiver
        const NetworkSelectionMode currentMode = currentNetworkMode;
        NetworkSelectionMode nextMode = NetworkSelectionMode::AUTO;

        switch (currentMode) {
          case NetworkSelectionMode::AUTO:
            nextMode = NetworkSelectionMode::MANUAL_HOME;
            break;
          case NetworkSelectionMode::MANUAL_HOME:
            nextMode = NetworkSelectionMode::MANUAL_WORK;
            break;
          case NetworkSelectionMode::MANUAL_WORK:
            nextMode = NetworkSelectionMode::AUTO;
            break;
          default:
            nextMode = NetworkSelectionMode::AUTO;
        }

        setNetworkMode(nextMode);

        // Show network mode change on display
        const char* modeStr = "Unknown";
        switch (nextMode) {
          case NetworkSelectionMode::AUTO:
            modeStr = "Auto";
            break;
          case NetworkSelectionMode::MANUAL_HOME:
            modeStr = "Home";
            break;
          case NetworkSelectionMode::MANUAL_WORK:
            modeStr = "Work";
            break;
          default:
            modeStr = "Auto";
        }

        oledMsg("Network Mode", modeStr);
        Serial.printf("Network mode changed to %s\n", modeStr);

        // Reconnect with new mode
        if (wifiConnected) {
          WiFi.disconnect();
          delay(1000);
          if (connectToWiFi()) {
            wifiConnected = true;
            oledMsg("Reconnected", getCurrentNetworkLocation());
          } else {
            wifiConnected = false;
            oledMsg("Reconnect", "Failed");
          }
        }
#else
        // For non-WiFi receivers, cycle SF instead
        currentSfIndex = (currentSfIndex + 1) % (sizeof(sfValues) / sizeof(sfValues[0]));
        currentSF = sfValues[currentSfIndex];
        updateRadioSettings();
        savePersistedSettings();
        Serial.printf("SF changed to %d\n", currentSF);
#endif
      }
    } else {
      // Long press - cycle BW
      if (isSender) {
        const int nextIndex = (currentBwIndex + 1) % (sizeof(bwValues) / sizeof(bwValues[0]));
        const float nextBW = bwValues[nextIndex];
        startConfigBroadcast(currentFreq, nextBW, currentSF, currentCR, currentTxPower);
        Serial.printf("BW change requested -> %.0f kHz (broadcasting to receiver)\n", nextBW);
      } else {
        currentBwIndex = (currentBwIndex + 1) % (sizeof(bwValues) / sizeof(bwValues[0]));
        currentBW = bwValues[currentBwIndex];
        updateRadioSettings();
        savePersistedSettings();
        Serial.printf("BW changed to %.0f kHz\n", currentBW);
      }
    }

    lastButtonMs = now;
  }

  lastButtonState = s;
}

// OTA Function Declarations
#ifdef ENABLE_WIFI_OTA
static void initWiFi();
static void initOTA();
static void triggerLoraFirmwareUpdates();
static bool storeCurrentFirmware();
#endif
static void handleLoraOtaPacket(const String& packet);
static void checkLoraOtaTimeout();
// Only receivers send firmware out
#ifdef ENABLE_WIFI_OTA
static void sendLoraOtaUpdate(const uint8_t* firmware, size_t firmwareSize);
#endif

// OLED Display Functions
static void drawStatusBar();

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("\n=== LtngDet LoRa + OLED (Heltec V3) ===");

  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // Initialize current values
  currentFreq = LORA_FREQ_MHZ;
  currentBW = LORA_BW_KHZ;
  currentSF = LORA_SF;
  currentCR = LORA_CR;
  currentTxPower = LORA_TX_DBM;

  // initial role from build flags
#ifdef ROLE_SENDER
  isSender = true;
#elif defined(ROLE_RECEIVER)
  isSender = false;
#else
  isSender = true;
#endif

  // Load persisted settings/role (overrides defaults when present)
  loadPersistedSettingsAndRole();
  computeIndicesFromCurrent();

  initDisplay();
  oledMsg("Booting...", "Heltec V3");
  oledRole();

  initRadioOrHalt();

  // Initialize WiFi and OTA for receivers
#ifdef ENABLE_WIFI_OTA
  if (!isSender) {
    initWiFi();
    if (wifiConnected) {
      initOTA();
      oledMsg("WiFi + OTA", "Ready");
    }
  }
#endif

  // Broadcast current settings at boot if sender
  if (isSender) {
    // Give receivers time to enter control-channel listen
    delay(750);
    // Also use the control channel to reach mismatched receivers
    broadcastConfigOnControlChannel(6, 250);
    startConfigBroadcast(currentFreq, currentBW, currentSF, currentCR, currentTxPower);
  }
  // Try to catch a control-channel config at boot if receiver
  if (!isSender) {
    tryReceiveConfigOnControlChannel(6000);
  }
}

void loop() {
  static uint32_t lastTxMs = 0;
  static uint32_t lastRxMs = 0;
  uint32_t now = millis();

  // Check button more frequently
  updateButton();

  if (isSender) {
    if (pendingConfigBroadcast) {
      if (now - lastTxMs >= 50 && now - cfgLastTxMs >= 300) {
        char msg[64];
        snprintf(msg, sizeof(msg), "CFG F=%.1f BW=%.0f SF=%d CR=%d TX=%d",
                 pendingFreq, pendingBW, pendingSF, pendingCR, pendingTxPower);
        int st = radio.transmit(msg);
        if (st == RADIOLIB_ERR_NONE) {
          Serial.printf("[TX] %s OK\n", msg);
        } else {
          Serial.printf("[TX] %s FAIL %d\n", msg, st);
        }
        cfgLastTxMs = now;
        cfgRemaining--;
        if (cfgRemaining <= 0) {
          // Apply the new settings on the transmitter after broadcasting
          currentFreq = pendingFreq;
          currentBW = pendingBW;
          currentSF = pendingSF;
          currentCR = pendingCR;
          currentTxPower = pendingTxPower;

          // Update index trackers to reflect applied settings
          for (size_t i = 0; i < (sizeof(sfValues) / sizeof(sfValues[0])); i++) {
            if (sfValues[i] == currentSF) { currentSfIndex = i; break; }
          }
          for (size_t i = 0; i < (sizeof(bwValues) / sizeof(bwValues[0])); i++) {
            if (bwValues[i] == currentBW) { currentBwIndex = i; break; }
          }
          for (size_t i = 0; i < (sizeof(txPowerValues) / sizeof(txPowerValues[0])); i++) {
            if (txPowerValues[i] == currentTxPower) { currentTxIndex = i; break; }
          }

          updateRadioSettings();
          savePersistedSettings();
          oledMsg("Sync complete", "TX switched");
          pendingConfigBroadcast = false;
          lastTxMs = now; // reset TX timer
        }
      }
    } else {
      // Non-blocking TX every 2 seconds
      if (now - lastTxMs >= 2000) {
        char msg[48];
        snprintf(msg, sizeof(msg), "PING seq=%lu", (unsigned long)seq++);
        int st = radio.transmit(msg);
        if (st == RADIOLIB_ERR_NONE) {
          Serial.printf("[TX] %s OK\n", msg);
          // Show ping on two lines
          unsigned long usedSeq = (unsigned long)(seq - 1);
          char seqLine[20]; snprintf(seqLine, sizeof(seqLine), "seq=%lu", usedSeq);
          oledMsg("PING", seqLine);
        } else {
          char e[24]; snprintf(e, sizeof(e), "err %d", st);
          Serial.printf("[TX] %s FAIL %s\n", msg, e);
          oledMsg("TX FAIL", msg, e);
        }
        lastTxMs = now;
      }
    }
  } else {
    // Non-blocking RX every 50ms
    if (now - lastRxMs >= 50) {
      String rx;
      int st = radio.receive(rx);
      if (st == RADIOLIB_ERR_NONE) {
        float rssi = radio.getRSSI();
        float snr  = radio.getSNR();

        // Update signal quality tracking
        lastRSSI = rssi;
        lastSNR = snr;
        lastPacketTime = now;
        packetCount++;

        if (rx.startsWith("CFG ")) {
          float nf = currentFreq;
          float nb = currentBW;
          int nsf = currentSF;
          int ncr = currentCR;
          int ntx = currentTxPower;
          int parsed = sscanf(rx.c_str(), "CFG F=%f BW=%f SF=%d CR=%d TX=%d", &nf, &nb, &nsf, &ncr, &ntx);
          if (parsed == 5) {
            currentFreq = nf;
            currentBW = nb;
            currentSF = nsf;
            currentCR = ncr;
            currentTxPower = ntx;

            // Update index trackers to reflect applied settings
            for (size_t i = 0; i < (sizeof(sfValues) / sizeof(sfValues[0])); i++) {
              if (sfValues[i] == currentSF) { currentSfIndex = i; break; }
            }
            for (size_t i = 0; i < (sizeof(bwValues) / sizeof(bwValues[0])); i++) {
              if (bwValues[i] == currentBW) { currentBwIndex = i; break; }
            }
            for (size_t i = 0; i < (sizeof(txPowerValues) / sizeof(txPowerValues[0])); i++) {
              if (txPowerValues[i] == currentTxPower) { currentTxIndex = i; break; }
            }

            updateRadioSettings();
            savePersistedSettings();
            char l2[20]; snprintf(l2, sizeof(l2), "RSSI %.1f", rssi);
            Serial.printf("[RX] APPLIED %s | SNR %.1f | PKT:%lu\n", rx.c_str(), snr, packetCount);
            oledMsg("SYNC", rx.c_str(), l2);
          } else {
            char l2[20]; snprintf(l2, sizeof(l2), "RSSI %.1f", rssi);
            Serial.printf("[RX] CFG PARSE FAIL | %s | SNR %.1f | PKT:%lu\n", rx.c_str(), snr, packetCount);
            oledMsg("RX", rx.c_str(), l2);
          }
        } else if (rx.startsWith("OTA_")) {
          // Handle OTA packets (both roles)
          handleLoraOtaPacket(rx);
        } else if (rx.startsWith("FW_UPDATE_AVAILABLE") || rx.startsWith("UPDATE_NOW")) {
          // Sender: request update when notified
          if (isSender) {
            Serial.println("FW update notice received; requesting update...");
            radio.transmit("REQUEST_UPDATE");
          }
        } else if (rx.startsWith("REQUEST_UPDATE")) {
          // Receiver only: handle update request from transmitter
          if (!isSender) {
            Serial.println("Transmitter requested firmware update!");
            oledMsg("Update Req", "Received");

            // Acknowledge the request
            radio.transmit("UPDATE_ACK");
            delay(100);

            // Send the actual firmware if we have it stored
            #ifdef ENABLE_WIFI_OTA
            if (hasStoredFirmware && storedFirmwareSize > 0) {
              Serial.printf("Sending stored firmware (%zu bytes) to transmitter\n", storedFirmwareSize);
              oledMsg("Sending FW", "To TX");
              sendLoraOtaUpdate(storedFirmware, storedFirmwareSize);
            } else {
              Serial.println("No firmware stored to send!");
              oledMsg("No FW", "Stored");
              radio.transmit("NO_FIRMWARE");
            }
            #else
            radio.transmit("NO_FIRMWARE");
            #endif
          }
        } else {
          char l2[20]; snprintf(l2, sizeof(l2), "RSSI %.1f", rssi);
          if (rx.startsWith("PING ")) {
            // Extract seq part from message "PING seq=NNN"
            const char* seqPtr = strstr(rx.c_str(), "seq=");
            const char* seqStr = seqPtr ? seqPtr : rx.c_str();
            oledMsg("PING", seqStr);
          } else {
            Serial.printf("[RX] %s | %s | SNR %.1f | PKT:%lu\n", rx.c_str(), l2, snr, packetCount);
            oledMsg("RX", rx.c_str(), l2);
          }
        }
      } else if (st != RADIOLIB_ERR_RX_TIMEOUT) {
        errorCount++;
        char e[24]; snprintf(e, sizeof(e), "err %d", st);
        Serial.printf("[RX] FAIL %s | ERR:%lu\n", e, errorCount);
        oledMsg("RX FAIL", e);
      }
      lastRxMs = now;
    }
  }

  // Handle OTA updates (WiFi OTA only on receiver)
  #ifdef ENABLE_WIFI_OTA
  if (!isSender && wifiConnected) {
    ArduinoOTA.handle();

    // Periodically check WiFi connection and reconnect if needed
    static uint32_t lastWiFiCheck = 0;
    if (now - lastWiFiCheck >= 30000) { // Check every 30 seconds
      if (!checkWiFiConnection()) {
        wifiConnected = false;
        oledMsg("WiFi", "Reconnecting...");
      } else if (!wifiConnected) {
        wifiConnected = true;
        oledMsg("WiFi", "Reconnected");
      }
      lastWiFiCheck = now;
    }
  }
  #endif

  // Check LoRa OTA timeout (both roles)
  checkLoraOtaTimeout();

  // Small delay to prevent overwhelming the system, but keep button responsive
  delay(10);
}

// WiFi and OTA Functions (Receiver only)
#ifdef ENABLE_WIFI_OTA
static void initWiFi() {
  Serial.println("Initializing WiFi...");
  oledMsg("WiFi", "Connecting...");

  // Initialize WiFi preferences
  initWiFiPreferences();

  // Print configured networks
  printConfiguredNetworks();

  // Attempt to connect using the WiFi manager
  if (connectToWiFi()) {
    wifiConnected = true;
    const char* location = getCurrentNetworkLocation();
    Serial.printf("\nWiFi connected to %s! IP: %s\n",
                  location, WiFi.localIP().toString().c_str());
    oledMsg("WiFi", location);
  } else {
    Serial.println("\nWiFi connection failed!");
    oledMsg("WiFi", "Failed!");
  }
}

static void initOTA() {
  if (!wifiConnected) return;

  ArduinoOTA.setHostname(OTA_HOSTNAME);
  ArduinoOTA.setPassword(OTA_PASSWORD);

  ArduinoOTA.onStart([]() {
    otaActive = true;
    Serial.println("OTA Update starting...");
    oledMsg("OTA", "Starting...");
  });

    ArduinoOTA.onEnd([]() {
    otaActive = false;
    Serial.println("OTA Update complete!");
    oledMsg("OTA", "Complete!");
    delay(1000);

    // NEW: Store the firmware for LoRa OTA cascade updates
    #ifdef ENABLE_WIFI_OTA
    if (!isSender) {
      // Store the current firmware for LoRa OTA distribution
      if (storeCurrentFirmware()) {
        Serial.println("Firmware stored for LoRa OTA distribution");
        oledMsg("Firmware", "Stored");
        delay(1000);

        // Now trigger LoRa firmware updates
        Serial.println("Triggering LoRa firmware updates...");
        oledMsg("LoRa Update", "Triggering...");
        triggerLoraFirmwareUpdates();
      } else {
        Serial.println("Failed to store firmware for LoRa OTA");
        oledMsg("Firmware", "Store failed");
        delay(1000);
      }
    }
    #endif
  });

  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    int percent = (progress * 100) / total;
    char progressStr[20];
    snprintf(progressStr, sizeof(progressStr), "%d%%", percent);
    oledMsg("OTA Update", progressStr);
  });

  ArduinoOTA.onError([](ota_error_t error) {
    otaActive = false;
    Serial.printf("OTA Error: %u\n", error);
    char errorStr[20];
    snprintf(errorStr, sizeof(errorStr), "Error: %u", error);
    oledMsg("OTA Error", errorStr);
  });

  ArduinoOTA.begin();
  Serial.println("OTA initialized");
}
#endif

// LoRa OTA Functions (both sender and receiver)
static void handleLoraOtaPacket(const String& packet) {
  if (packet.startsWith("OTA_START:")) {
    // Format: OTA_START:size:chunks
    int parsed = sscanf(packet.c_str(), "OTA_START:%u:%u", &loraOtaExpectedSize, &loraOtaTimeout);
    if (parsed == 2) {
      loraOtaActive = true;
      loraOtaStartTime = millis();
      loraOtaReceivedSize = 0;
      loraOtaBufferSize = 0;

      Serial.printf("LoRa OTA starting: %lu bytes\n", loraOtaExpectedSize);
      oledMsg("LoRa OTA", "Starting...");
    }
  } else if (packet.startsWith("OTA_DATA:")) {
    if (!loraOtaActive) return;

    // Format: OTA_DATA:chunk:data
    int chunk;
    char data[256];
    int parsed = sscanf(packet.c_str(), "OTA_DATA:%d:%255s", &chunk, data);
    if (parsed == 2) {
      // Decode base64 data and add to buffer
      // For simplicity, we'll use a basic approach
      size_t dataLen = strlen(data);
      if (loraOtaBufferSize + dataLen < sizeof(loraOtaBuffer)) {
        memcpy(loraOtaBuffer + loraOtaBufferSize, data, dataLen);
        loraOtaBufferSize += dataLen;
        loraOtaReceivedSize += dataLen;

        int percent = (loraOtaReceivedSize * 100) / loraOtaExpectedSize;
        char progressStr[20];
        snprintf(progressStr, sizeof(progressStr), "%d%%", percent);
        oledMsg("LoRa OTA", progressStr);
      }
    }
  } else if (packet.startsWith("OTA_END:")) {
    if (!loraOtaActive) return;

    // Verify we have all data
    if (loraOtaReceivedSize >= loraOtaExpectedSize) {
      Serial.println("LoRa OTA complete, flashing...");
      oledMsg("LoRa OTA", "Flashing...");

      // Flash the firmware
      if (Update.begin(loraOtaExpectedSize)) {
        (void)Update.write(loraOtaBuffer, loraOtaBufferSize); // Suppress unused variable warning
        if (Update.end()) {
          Serial.println("Firmware flashed successfully!");
          oledMsg("OTA Complete", "Rebooting...");
          delay(2000);
          ESP.restart();
        } else {
          Serial.println("Firmware flash failed!");
          oledMsg("OTA Error", "Flash failed!");
        }
      } else {
        Serial.println("OTA begin failed!");
        oledMsg("OTA Error", "Begin failed!");
      }
    }

    loraOtaActive = false;
  }
}

static void checkLoraOtaTimeout() {
  if (loraOtaActive && (millis() - loraOtaStartTime > loraOtaTimeout)) {
    Serial.println("LoRa OTA timeout!");
    oledMsg("LoRa OTA", "Timeout!");
    loraOtaActive = false;
  }
}

// Function to send OTA update to transmitters (receiver only)
#ifdef ENABLE_WIFI_OTA
static void sendLoraOtaUpdate(const uint8_t* firmware, size_t firmwareSize) {
  if (isSender) return; // Only receivers can send OTA updates

  Serial.printf("Sending LoRa OTA update: %zu bytes\n", firmwareSize);
  oledMsg("LoRa OTA", "Sending...");

  // Send OTA start packet
  char startMsg[64];
  snprintf(startMsg, sizeof(startMsg), "OTA_START:%zu:%lu", firmwareSize, loraOtaTimeout);
  radio.transmit(startMsg);
  delay(100);

  // Break firmware into chunks and send
  const size_t chunkSize = 200; // Max LoRa packet size
  size_t sentBytes = 0;
  int chunkNum = 0;

  while (sentBytes < firmwareSize) {
    size_t currentChunkSize = min(chunkSize, firmwareSize - sentBytes);

    // Create chunk packet
    char chunkMsg[256];
    snprintf(chunkMsg, sizeof(chunkMsg), "OTA_DATA:%d:", chunkNum);

    // Copy chunk data (simplified - in practice you'd want base64 encoding)
    memcpy(chunkMsg + strlen(chunkMsg), firmware + sentBytes, currentChunkSize);
    chunkMsg[strlen(chunkMsg) + currentChunkSize] = '\0';

    // Send chunk
    radio.transmit(chunkMsg);
    delay(50);

    sentBytes += currentChunkSize;
    chunkNum++;

    // Update progress
    int percent = (sentBytes * 100) / firmwareSize;
    char progressStr[20];
    snprintf(progressStr, sizeof(progressStr), "Sending %d%%", percent);
    oledMsg("LoRa OTA", progressStr);
  }

  // Send OTA end packet
  radio.transmit("OTA_END:");
  delay(100);

  Serial.println("LoRa OTA update sent!");
  oledMsg("LoRa OTA", "Sent!");
}

// NEW: Function to automatically trigger LoRa firmware updates after WiFi OTA
static void triggerLoraFirmwareUpdates() {
  if (isSender) return; // Only receivers can trigger updates

  Serial.println("Broadcasting firmware update notification...");
  oledMsg("LoRa Update", "Broadcasting...");

  // Proactively resync receivers to our current settings over control channel
  // to maximize the chance they can hear the update notifications
  broadcastConfigOnControlChannel(8, 250);

  // Send multiple notifications to ensure transmitters receive them
  for (int i = 0; i < 10; i++) {
    // Send firmware update available notification
    radio.transmit("FW_UPDATE_AVAILABLE");
    delay(200);

          // Send version info from stored firmware
      char versionMsg[64];
      if (hasStoredFirmware) {
        snprintf(versionMsg, sizeof(versionMsg), "FW_VERSION:%lu", firmwareVersion);
      } else {
        snprintf(versionMsg, sizeof(versionMsg), "FW_VERSION:0.0.0");
      }
      radio.transmit(versionMsg);
      delay(200);

    // Send update trigger command
    radio.transmit("UPDATE_NOW");
    delay(200);
  }

  Serial.println("Firmware update notifications sent!");
  oledMsg("LoRa Update", "Notifications sent!");

  // Wait a bit for transmitters to process
  delay(2000);

  // Check if any transmitters requested updates
  Serial.println("Checking for update requests...");
  oledMsg("LoRa Update", "Checking...");

  // Listen for update requests for longer to catch remote nodes
  uint32_t startTime = millis();
  while (millis() - startTime < 15000) { // Listen for 15 seconds
    String rx;
    int r = radio.receive(rx);
    if (r == RADIOLIB_ERR_NONE) {
      if (rx.startsWith("REQUEST_UPDATE")) {
        Serial.println("Transmitter requested update!");
        oledMsg("LoRa Update", "Request received!");

        // Here you could implement logic to send the actual firmware
        // For now, we'll just acknowledge the request
        radio.transmit("UPDATE_ACK");
        delay(100);

        // You could call sendLoraOtaUpdate() here with the firmware data
        // sendLoraOtaUpdate(firmwareData, firmwareSize);
      }
    }
    delay(100);
  }

  Serial.println("LoRa firmware update trigger complete!");
  oledMsg("LoRa Update", "Complete!");
}

// NEW: Function to store the current firmware for LoRa OTA distribution
static bool storeCurrentFirmware() {
  if (isSender) return false; // Only receivers can store firmware

  // For now, we'll create a simple firmware representation
  // In a real implementation, you'd read the actual firmware from flash
  // or store it during the WiFi OTA process

  // Create a simple firmware header
  const char* firmwareHeader = "LtngDet_FW_v1.0.0";
  size_t headerLen = strlen(firmwareHeader);

  // Copy header to firmware buffer
  memcpy(storedFirmware, firmwareHeader, headerLen);

  // Add some dummy firmware data (in practice, this would be the actual firmware)
  // For demonstration, we'll create a simple firmware representation
  uint8_t dummyFirmware[] = {
    0xAA, 0x55, 0xAA, 0x55, // Magic bytes
    0x01, 0x00, 0x00,       // Version 1.0.0
    0x00, 0x00, 0x00, 0x00, // Checksum placeholder
    0x00, 0x00, 0x00, 0x00  // Size placeholder
  };

  size_t dummyLen = sizeof(dummyFirmware);
  memcpy(storedFirmware + headerLen, dummyFirmware, dummyLen);

  // Set the total firmware size
  storedFirmwareSize = headerLen + dummyLen;
  hasStoredFirmware = true;

  Serial.printf("Firmware stored: %zu bytes (header: %zu, data: %zu)\n",
                storedFirmwareSize, headerLen, dummyLen);

  // TODO: In a real implementation, you would:
  // 1. Read the actual firmware from flash memory
  // 2. Calculate proper checksums
  // 3. Store the complete firmware binary
  // 4. Handle firmware versioning properly

  return true;
}
#endif
