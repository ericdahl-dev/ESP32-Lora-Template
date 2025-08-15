#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include "wifi_config.h"

// Initialize WiFi preferences and load saved settings
void initWiFiPreferences();

// Main WiFi connection function with automatic fallback
bool connectToWiFi();

// Connect to a specific network by index
bool connectToSpecificNetwork(int networkIndex);

// Set network selection mode
void setNetworkMode(NetworkSelectionMode mode);

// Get current network location string
const char* getCurrentNetworkLocation();

// Get current network index
int getCurrentNetworkIndex();

// Check if WiFi is connected and reconnect if needed
bool checkWiFiConnection();

// Get current WiFi status string for display
const char* getWiFiStatusString();

// Print all configured networks to Serial
void printConfiguredNetworks();

// Network-specific connection functions
bool connectToHomeNetwork();
bool connectToWorkNetwork();

// Helper functions for automatic fallback
bool connectWithAutoFallback();

#endif // WIFI_MANAGER_H
