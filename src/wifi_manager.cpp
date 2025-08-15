#include "wifi_config.h"
#include <WiFi.h>
#include <Preferences.h>

// Global variables
NetworkSelectionMode currentNetworkMode = NetworkSelectionMode::AUTO;
static int currentConnectedNetworkIndex = -1;
static Preferences wifiPrefs;

// Initialize WiFi preferences
void initWiFiPreferences() {
  wifiPrefs.begin("WiFiConfig", false);

  // Load saved network mode
  int savedMode = wifiPrefs.getInt("networkMode", (int)NetworkSelectionMode::AUTO);
  currentNetworkMode = static_cast<NetworkSelectionMode>(savedMode);

  // Load last connected network index
  currentConnectedNetworkIndex = wifiPrefs.getInt("lastNetwork", -1);

  Serial.printf("WiFi Manager: Loaded mode %d, last network %d\n",
                (int)currentNetworkMode, currentConnectedNetworkIndex);
}

// Save current network mode to preferences
void saveNetworkMode() {
  wifiPrefs.putInt("networkMode", (int)currentNetworkMode);
  wifiPrefs.putInt("lastNetwork", currentConnectedNetworkIndex);
}

// Get current network location string
const char* getCurrentNetworkLocation() {
  if (currentConnectedNetworkIndex >= 0 && currentConnectedNetworkIndex < NUM_WIFI_NETWORKS) {
    return WIFI_NETWORKS[currentConnectedNetworkIndex].location;
  }
  return "Unknown";
}

// Get current network index
int getCurrentNetworkIndex() {
  return currentConnectedNetworkIndex;
}

// Connect to a specific network by index
bool connectToSpecificNetwork(int networkIndex) {
  if (networkIndex < 0 || networkIndex >= NUM_WIFI_NETWORKS) {
    Serial.printf("WiFi Manager: Invalid network index %d\n", networkIndex);
    return false;
  }

  const WiFiNetwork& network = WIFI_NETWORKS[networkIndex];
  Serial.printf("WiFi Manager: Attempting to connect to %s (%s)...\n",
                network.ssid, network.location);

  WiFi.begin(network.ssid, network.password);

  uint32_t startTime = millis();
  while (WiFi.status() != WL_CONNECTED &&
         (millis() - startTime) < WIFI_CONNECT_TIMEOUT_MS) {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    currentConnectedNetworkIndex = networkIndex;
    saveNetworkMode();
    Serial.printf("\nWiFi Manager: Connected to %s (%s) - IP: %s\n",
                  network.ssid, network.location, WiFi.localIP().toString().c_str());
    return true;
  } else {
    Serial.printf("\nWiFi Manager: Failed to connect to %s\n", network.ssid);
    return false;
  }
}

// Connect with automatic fallback through all networks
bool connectWithAutoFallback() {
  Serial.println("WiFi Manager: Auto mode - trying networks in priority order...");

  // First, try the last successfully connected network
  if (currentConnectedNetworkIndex >= 0 && currentConnectedNetworkIndex < NUM_WIFI_NETWORKS) {
    Serial.printf("WiFi Manager: Trying last known network %d first\n", currentConnectedNetworkIndex);
    if (connectToSpecificNetwork(currentConnectedNetworkIndex)) {
      return true;
    }
  }

  // Try networks in priority order
  for (int attempt = 0; attempt < NUM_WIFI_NETWORKS; attempt++) {
    // Find network with this priority
    for (int i = 0; i < NUM_WIFI_NETWORKS; i++) {
      if (WIFI_NETWORKS[i].priority == (attempt + 1)) {
        Serial.printf("WiFi Manager: Trying priority %d network: %s\n",
                      attempt + 1, WIFI_NETWORKS[i].ssid);

        if (connectToSpecificNetwork(i)) {
          return true;
        }
        break;
      }
    }

    // Wait before trying next priority level
    if (attempt < NUM_WIFI_NETWORKS - 1) {
      delay(WIFI_RETRY_DELAY_MS);
    }
  }

  Serial.println("WiFi Manager: All networks failed");
  return false;
}

// Connect specifically to home network
bool connectToHomeNetwork() {
  Serial.println("WiFi Manager: Manual mode - connecting to home network...");

  for (int i = 0; i < NUM_WIFI_NETWORKS; i++) {
    if (strcmp(WIFI_NETWORKS[i].location, "Home") == 0) {
      return connectToSpecificNetwork(i);
    }
  }

  Serial.println("WiFi Manager: No home network configured");
  return false;
}

// Connect specifically to work network
bool connectToWorkNetwork() {
  Serial.println("WiFi Manager: Manual mode - connecting to work network...");

  for (int i = 0; i < NUM_WIFI_NETWORKS; i++) {
    if (strcmp(WIFI_NETWORKS[i].location, "Work") == 0) {
      return connectToSpecificNetwork(i);
    }
  }

  Serial.println("WiFi Manager: No work network configured");
  return false;
}

// Main WiFi connection function with automatic fallback
bool connectToWiFi() {
  Serial.println("WiFi Manager: Starting WiFi connection...");

  // Initialize WiFi
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  // Try to connect based on current mode
  switch (currentNetworkMode) {
    case NetworkSelectionMode::AUTO:
      return connectWithAutoFallback();

    case NetworkSelectionMode::MANUAL_HOME:
      return connectToHomeNetwork();

    case NetworkSelectionMode::MANUAL_WORK:
      return connectToWorkNetwork();

    case NetworkSelectionMode::MANUAL_CUSTOM:
      // Use last connected network or first available
      if (currentConnectedNetworkIndex >= 0 && currentConnectedNetworkIndex < NUM_WIFI_NETWORKS) {
        return connectToSpecificNetwork(currentConnectedNetworkIndex);
      } else {
        return connectToSpecificNetwork(0);
      }

    default:
      return connectWithAutoFallback();
  }
}

// Set network selection mode
void setNetworkMode(NetworkSelectionMode mode) {
  if (currentNetworkMode != mode) {
    currentNetworkMode = mode;
    saveNetworkMode();
    Serial.printf("WiFi Manager: Network mode changed to %d\n", (int)mode);

    // If currently connected, reconnect with new mode
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("WiFi Manager: Reconnecting with new mode...");
      WiFi.disconnect();
      delay(1000);
      connectToWiFi();
    }
  }
}

// Check if WiFi is connected and reconnect if needed
bool checkWiFiConnection() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi Manager: Connection lost, attempting to reconnect...");
    return connectToWiFi();
  }
  return true;
}

// Get current WiFi status string
const char* getWiFiStatusString() {
  if (currentConnectedNetworkIndex >= 0 && currentConnectedNetworkIndex < NUM_WIFI_NETWORKS) {
    const WiFiNetwork& network = WIFI_NETWORKS[currentConnectedNetworkIndex];
    static char statusStr[32];
    snprintf(statusStr, sizeof(statusStr), "%s", network.location);
    return statusStr;
  }
  return "Disconnected";
}

// Print all configured networks
void printConfiguredNetworks() {
  Serial.println("WiFi Manager: Configured networks:");
  for (int i = 0; i < NUM_WIFI_NETWORKS; i++) {
    const WiFiNetwork& network = WIFI_NETWORKS[i];
    Serial.printf("  [%d] %s (%s) - Priority %d\n",
                  i, network.ssid, network.location, network.priority);
  }
}
