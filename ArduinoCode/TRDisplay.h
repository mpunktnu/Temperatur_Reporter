//TRDisplay.h
#ifndef TRDISPLAY_H
#define TRDISPLAY_H

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#ifndef OLED_RESET
#define OLED_RESET 0
#endif //OLED_RESET

//different viewing options for the main screen of the OLED
enum MainScreenMode {
  Standard,
  MoreInfo
};

class TRDisplay {
  public :
    TRDisplay();

    void bootScreen();
    void connectingScreen(const String ssid);
    void APScreen(bool networkSaved, String ssid);
    void mainScreen(int signalStrength, const float* temperatures, int sensorCount, int pin); 
    
    void printDot();
    void switchScreenMode();
    bool isInScreenMode(MainScreenMode screenMode);
    MainScreenMode getScreenMode();
    
  private :
    void printSignalStrength(int signalStrength);
    void printTemperature(float temperature); 
    void noSensorsError();
    void screenStateStandard();
    void screenStateMoreInfo();

    //local copies of the variables to be displayed
    const float* _temperatures;
    float _lowestTemperature;
    int _signalStrength;
    int _sensorCount;
    int _pin;

    //keeps track of which screen mode the main screen is in
    MainScreenMode _screenMode = Standard;

    //function pointer _screenState that points to the function handling the current screenMode
    //the function it points to must be of return typ void and take no arguments (void)
    void (TRDisplay::*_screenState) (void);

    //object communicating with the OLED
    Adafruit_SSD1306 display{Adafruit_SSD1306(OLED_RESET)};
};

#endif //TRDISPLAY_H
