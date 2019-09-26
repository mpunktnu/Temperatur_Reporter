//TRDisplay.cpp
#include "TRDisplay.h"
#include <Arduino.h>

TRDisplay::TRDisplay() {
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.setTextSize(1);
  display.setTextColor(WHITE);
  _screenState = &TRDisplay::screenStateStandard;
}

void TRDisplay::bootScreen() {
  display.clearDisplay();
  display.setCursor(0,0); 
  display.println("Startar");  
  display.display();
}

void TRDisplay::connectingScreen(const String ssid) {
  display.clearDisplay();
  display.setCursor(0,0); 
  display.println("Ansluter\ntill:");
  display.print(ssid);  
  display.display();
}

void TRDisplay::APScreen(bool networkSaved, const String ssid) {
  display.clearDisplay();
  display.setCursor(0,0);
  
  if(networkSaved)
    display.print("Kunde inte\nansluta.\n");
  else
    display.print("Inget WiFi\nsparat.\n");
  
  display.println("Anslut\ntill:");
  display.println(ssid);
  display.display();
}

void TRDisplay::printDot() {
  display.print(".");
  display.display();
}

void TRDisplay::mainScreen(int signalStrength, const float* temperatures, int sensorCount, int pin) { 
  //display error message and return if no sensors are connected
  if(sensorCount == 0) {
    noSensorsError();
    return;
  } 
  _signalStrength = signalStrength;
  _temperatures = temperatures;
  _sensorCount = sensorCount;
  _pin = pin;
  
  //if sensor count == 2 {lowest temp = min of two values} else {lowest temp = first temp}
  _lowestTemperature = (_sensorCount == 2) ? min(_temperatures[0], _temperatures[1]) : _temperatures[0];
  
  (this->*_screenState) ();
}

//prints only the lowest temp in large font
void TRDisplay::screenStateStandard() {
    if(_lowestTemperature == -127 || _lowestTemperature == 85)
      return;
  
    display.clearDisplay();
    display.setCursor(0,0);
    display.setTextSize(1);
    display.print("\n\n");
    display.setTextSize(2);

    //right alignment
    if(_lowestTemperature > 0)
      display.print(" ");
      
    char tempStr[6];
    sprintf(tempStr, "%0.1f", _lowestTemperature);
    display.println(tempStr);
    display.setTextSize(1);
    display.print("\n        ");
    display.print("\xF7" "C"); //degree sign and C afterwards
    display.display();
}

//prints wifi strength, pin, two temperatures and sensor count
void TRDisplay::screenStateMoreInfo() {
  display.clearDisplay();
  display.setCursor(0,0);
  printSignalStrength(_signalStrength);
  display.println("Pin:");
  display.println(_pin);
  printTemperature(_temperatures[0]);
  printTemperature(_temperatures[1]);
  display.println("Sensorer:" + String(_sensorCount));
  display.display();
}

//prints error message for when no sensors are connected 
void TRDisplay::noSensorsError() {
  display.clearDisplay();
  display.setCursor(0,0);
  display.setTextSize(2);
  display.println("FEL");
  display.setTextSize(1);
  display.print("Ingen\nsensor\nansluten.");
  display.display();
}

//prints temperature on one line as 23.5°C
void TRDisplay::printTemperature(float temperature) {
  //85 if sensor cant be read, -127 if no sensor found
  //just print a new line
  if(temperature == 85 || temperature == -127) {
    display.println("");
    return;
  }
  
  char tempStr[9];
  sprintf(tempStr, "%0.1f %cC", temperature, (char)247);
  display.println(tempStr);
}

//display wifi signal strenght using 4 bars
void TRDisplay::printSignalStrength(int signalStrength) {
  display.println("WiFi: ");
  
  //draw rectangles to represent wifi signal strength
  //hardcoded to top right corner of a 64×48 display
  //the rectangels are 6 pixels wide with 1 pixel in between
  for (int i = 0; i < signalStrength; i++) {
    //pos x, pos y, width, height, color
    display.fillRect(61-(7*(4-i)), 2*(4-(i+1)), 6, 2*(i+1), WHITE);
  }
}

//switches screenMode of the mainscreen
void  TRDisplay::switchScreenMode() {  
  if (isInScreenMode(Standard)) {
    _screenMode = MoreInfo;
    _screenState = &TRDisplay::screenStateMoreInfo;
    Serial.println("ScreenMode:::::More Info");
  }
  else if (isInScreenMode(MoreInfo)) {
    _screenMode = Standard;
    _screenState = &TRDisplay::screenStateStandard;
    Serial.println("ScreenMode:::::Standard");
  }
}

bool TRDisplay::isInScreenMode(MainScreenMode screenMode) {
  return _screenMode == screenMode;
}

MainScreenMode TRDisplay::getScreenMode() {
  return _screenMode;
}

