// WiFi Networks Configuration Example
// Copy this file to wifi_networks.h and modify with your actual credentials
// Then include it in wifi_config.h

#ifndef WIFI_NETWORKS_EXAMPLE_H
#define WIFI_NETWORKS_EXAMPLE_H

// Example network configurations - modify these with your actual networks
static const WiFiNetwork wifiNetworks[] = {
  // Primary network (home) - highest priority
  {"YourHomeSSID", "YourHomePassword", "Home", 1},

  // Secondary network (work) - lower priority
  {"YourWorkSSID", "YourWorkPassword", "Work", 2},

  // Additional networks can be added here
  // {"Network3", "Password3", "Location3", 3},
  // {"Network4", "Password4", "Location4", 4},
};

// Instructions:
// 1. Replace "YourHomeSSID" with your actual home WiFi network name
// 2. Replace "YourHomePassword" with your actual home WiFi password
// 3. Replace "YourWorkSSID" with your actual work WiFi network name
// 4. Replace "YourWorkPassword" with your actual work WiFi password
// 5. Modify the location names if desired (e.g., "Office" instead of "Work")
// 6. Adjust priorities if needed (lower number = higher priority)
// 7. Add more networks as needed
// 8. Save this file as wifi_networks.h
// 9. Include it in wifi_config.h

#endif // WIFI_NETWORKS_EXAMPLE_H
