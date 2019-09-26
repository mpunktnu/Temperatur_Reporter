#ifndef TRESP_H
#define TRESP_H

#include "src/WifiManager_20190211/WiFiManager.h" // https://github.com/tzapu/WiFiManager
#include <ESP8266HTTPClient.h>

//namespace because there is no need for an instance
//but still want to categorise the code
//stands for TemperaturReporterESP
namespace TRESP {
  //prints chip ID (PIN) and MAC of the ESP
  void printESPInfo();
  
  //connects to saved network, or sets up an AP. Reboots unit if it fails
  void setupWiFi(const char* APName, void (*apCallback)(WiFiManager*), void (*connectionCallback)(String));
  
  //sends the temperature to temperatur.nu
  bool sendDataToWeb(float temperature, int sensorCount);
  
  //returns signal strength of connected network as 0-100
  int getRSSIasPercentage();

  //reboots ESP if it is not connected to a network
  void checkWiFiStatus();

  //returns true if there is a WiFI AP SSID saved
  bool networkIsSaved();

  //returns the ssid of the saved network
  String SSID();
}

#endif //TRESP8266_H
