#include "TRESP.h"
#include <Arduino.h>

namespace TRESP {
  
  void printESPInfo() {
    Serial.print("ESP CHIP ID: ");
    Serial.println(ESP.getChipId()); //PIN
    Serial.print("ESP MAC ADDRESS: ");
    Serial.println(WiFi.macAddress());
  }
  
  void setupWiFi(const char* APName, void (*apCallback)(WiFiManager*), void (*connectionCallback)(String)) {
    //wifi manager object that handles autoconnection to wifi and sets up config portal
    WiFiManager wifiManager;
    wifiManager.setHostname("Temperatur_"+ESP.getChipId());
    //deletes saved network, for Debug purposes
    //wifiManager.resetSettings(); 

    //tell autoConnect to stop blocking even if configuration was unsuccessful
    //so that the that the wemos can reboot 
    wifiManager.setBreakAfterConfig(true);

    //the function apCallback will be called when the ESP enters AP mode
    wifiManager.setAPCallback(apCallback);

    wifiManager.setConnectCallback(connectionCallback);

    //if there is a saved network, 
    //only setup the config portal for 5 minutes 
    //so that the unit wont stay in AP mode forever if there is a saved network
    if(wifiManager.getWiFiIsSaved())
      wifiManager.setConfigPortalTimeout(5 * 60); //5 min

    //wifimanager will attempt to connect to saved network,
    //if it fails it will host the config Portal
    wifiManager.autoConnect(APName);

    //if it fails to connect to network, reboot
    checkWiFiStatus();
  }

  //sends PIN, temperature, sensorCount, and amount of succesful uploads
  //via url string to temperatur.nu
  //returns true if connection was succesful and file was found at server
  bool sendDataToWeb(float temperature, int sensorCount) {
    Serial.println("--------TRESP::sendDataToWeb--------");
    
    //if no wifi connection, reboot
    checkWiFiStatus();
    
    //disable watchdog so that the ESP wont reboot itself
    //if it takes to long time to connect to the server
    ESP.wdtDisable();

    //keeps track of succesful uploads since boot
    //starts at 1 so that the first succesful upload will be 1
    static unsigned long succesfulUploads = 1;

    //the url that the ESP will try to reach, which contains the gathered data
    String url = "http://report.temperatur.nu/rapportera_v2.php?pin=" 
      +         String(ESP.getChipId()) //pin
      + "&t=" + String(temperature) //temp
      + "&n=" + String(sensorCount) //sensors plugged in
      + "&u=" + String(succesfulUploads); //succesful uploads since boot

    Serial.println(WiFi.localIP());
    HTTPClient http;
    Serial.print("[HTTP] begin...\n");
    Serial.println(url);   
    http.begin(url);
    
    // start connection and send HTTP header
    Serial.print("[HTTP] GET...\n");
    int httpCode = http.GET();
        
    //the bool that is returned, set to true if file was found at server
    bool success = false;
    
    // httpCode will be negative on error
    if (httpCode > 0) {
      // HTTP header has been send and Server response header has been handled
      Serial.printf("[HTTP] GET... code: %d\n", httpCode);
    
      // file found at server
      if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        Serial.println(payload);
        succesfulUploads++;
        success = true;
      }
    } else {
      Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
  
    http.end();
    ESP.wdtEnable(2000);
    return success;
  }
  
  int getRSSIasPercentage() {
    int rssi = WiFi.RSSI();
    int quality = 0;
  
    if (rssi <= -100) {
      quality = 0;
    } else if (rssi >= -50) {
      quality = 100;
    } else {
      quality = 2 * (rssi + 100);
    }
    return quality;
  }

  void checkWiFiStatus() {
    if(WiFi.status() != WL_CONNECTED) {
      ESP.restart();
      delay(1000);
    }
  }

  bool networkIsSaved() {
    return WiFi.SSID() != "";
  }

  String SSID() {
    return WiFi.SSID();
  }
} //TRESP8266
