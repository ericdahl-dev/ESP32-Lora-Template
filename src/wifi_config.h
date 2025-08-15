#ifndef WIFI_CONFIG_H
#define WIFI_CONFIG_H

// Multi-Network WiFi Configuration
// Supports multiple networks with automatic fallback

// Network Configuration Structure
struct WiFiNetwork {
  const char* ssid;
  const char* password;
  const char* location;  // "Home", "Work", etc.
  int priority;          // Lower number = higher priority
};

// Define your WiFi networks here
// Add or modify networks as needed
static const WiFiNetwork WIFI_NETWORKS[] = {
  {"Skeyelab", "diamond2", "Home", 1},      // Primary network (home)
  {"ND-guest", "", "Work", 2},  // Secondary network (work)
  // Add more networks as needed:
  // {"Network3", "password3", "Location3", 3},
};

// Number of configured networks
#define NUM_WIFI_NETWORKS (sizeof(WIFI_NETWORKS) / sizeof(WIFI_NETWORKS[0]))

// OTA Configuration
#define OTA_HOSTNAME "LtngDet-Receiver"
#define OTA_PASSWORD "123456"  // Password for OTA updates

// WiFi Connection Settings
#define WIFI_CONNECT_TIMEOUT_MS 10000    // 10 seconds per network
#define WIFI_RETRY_DELAY_MS 1000        // 1 second between retries
#define WIFI_MAX_RETRIES 3              // Max retries per network

// Network Selection Modes
enum class NetworkSelectionMode {
  AUTO,           // Automatic priority-based selection
  MANUAL_HOME,    // Force home network
  MANUAL_WORK,    // Force work network
  MANUAL_CUSTOM   // Force specific network by index
};

// Current network selection mode (stored in preferences)
extern NetworkSelectionMode currentNetworkMode;

// Function declarations
bool connectToWiFi();
bool connectToSpecificNetwork(int networkIndex);
void setNetworkMode(NetworkSelectionMode mode);
const char* getCurrentNetworkLocation();
int getCurrentNetworkIndex();

#endif // WIFI_CONFIG_H
