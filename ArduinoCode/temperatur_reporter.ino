//temperatur_reporter.ino

//A program that reads values of up to two temperature sensors
//and sends the lowest value to temperatur.nu
//it also displays information on a OLED screen
//Hardware: 
//Wemos D1 mini
//Wemos D1 mini OLED shield
//DS18B20+ temperature sensors

/* Libraries needed to be installed
 * OneWire
 * DallasTemperature
 * Adafruit_SSD1306_Wemos_Mini_OLED
 * Adafruit_GFX_Library
 * ESP8266WiFi https://github.com/esp8266/Arduino
 */

//includes
#include "TRESP.h" //ESP/WiFi functionality
#include "TRDisplay.h" //OLED print functionality
#include <DallasTemperature.h> //Handles reading of temp sensors
#include <Ticker.h> //used to call functions periodicly

//defines
#define ONE_WIRE_BUS D5 //Pin of temperature sensors
#define TEMPERATURE_PRECISION 12 //Number of bits used to store temperature value, 12 = MAX
#define OLED_BUTTON D7 //Pin connected to right button on OLED shield

//temperature objects
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

//TemperaturReporterDisplay object, where all the defined OLED printfunctions are
TRDisplay display;

//seconds in between each OLED main screen refresh
const unsigned int MAINSCREENUPDATEINTERVAL = 20;  //1 second
//seconds in between each upload
const unsigned int DATASENDINTERVAL = 3 * 60; //5 minutes
//if the last upload failed, retry with a shorter time
const unsigned int DATASENDSHORTINTERVAL = 1 * 60; //1 minute

//ssid of the access point that the ESP hosts when setting up the config portal
const char* APNAME = "Temperatur_reporter";

//remembers if last upload succeded
bool lastUploadSucceded = true;

 //last time the mainscreen was updated
  static unsigned long lastFrameTime = 0;
  //time of last upload
  static unsigned long lastUploadTime = 0;


//Ticker object to print dots while connecting/booting
Ticker printDotTicker;

void setup() {
  Serial.begin(115200);

  //display startmessege on OLED screen
  display.bootScreen();
  
  //calls printDot() 1 second in between
  printDotTicker.attach(1, printDot);

  //print ESP pin and mac to Serial port
  TRESP::printESPInfo();

  //connect to wifi/setup config portal with given name
  //and call APCallback() function when hosting an AP
  TRESP::setupWiFi(APNAME, APCallback, connectingCallback);

  //stop printing dots if connection was successful
  printDotTicker.detach();

  //tell sensors to start measuring
  sensors.begin();
  //set temperature bit resolution
  sensors.setResolution(TEMPERATURE_PRECISION);

  //set pin OLED_BUTTON as input, to work as a button
  pinMode(OLED_BUTTON, INPUT);
    mainScreen();
  sendDataToWeb();
  
  
}

void loop() {  
  //keep in mind static variables remain until program shuts down
  //and are only initialized upon declaration

  //last time the mainscreen was updated
 // static unsigned long lastFrameTime = millis();
  //time of last upload
//  static unsigned long lastUploadTime = lastFrameTime;

  //check if enough time has passed to update OLED screen
  if(timerDone(&lastFrameTime, MAINSCREENUPDATEINTERVAL))
    mainScreen();

  //check if enough time has passed to send temperature to web
  //if last upload succeded use DATASENDINTERVAL, else DATASENDSHORTINTERVAL
  if(timerDone(&lastUploadTime, lastUploadSucceded ? DATASENDINTERVAL : DATASENDSHORTINTERVAL))
    sendDataToWeb();

  //check if OLED_BUTTON was pressed, if so switch screenMode
  if(pressedSwitchButton()) {
    display.switchScreenMode();
    
    //directly call mainScreen() without waiting for timer, 
    //for better response when switching screenMode
    //though reading the values of the sensors may still result in some lag
    mainScreen();
  }
}

//when the ESP creates an accesspoint/starts config portal this function will be called
void APCallback(WiFiManager *myWiFiManager) {
  //stop printing dots
  printDotTicker.detach();
  
  //print information on OLED screen
  display.APScreen(myWiFiManager->getWiFiIsSaved(), myWiFiManager->getConfigPortalSSID());
}

void connectingCallback(String ssid) {
  //stop printing dots to avoid wierd multiple calls of printDot
  printDotTicker.detach();
  
  display.connectingScreen(ssid);
  
  //calls printDot() 1 second in between
  printDotTicker.attach(1, printDot);
}

//gathers the data to be sent to temperatur.nu
void sendDataToWeb() {
  //needs to call begin() in order to update device count
  sensors.begin();
Serial.print("Number of sensors");
Serial.println(sensors.getDeviceCount());
  //gather temperature and sensor count information
  //and pass it on to function sending it to temperatur.nu
  lastUploadSucceded = TRESP::sendDataToWeb(getLowestTemperature(), sensors.getDeviceCount());
  if(lastUploadSucceded) {
    Serial.print("Upload succeded, next upload will be in ");
    Serial.print(DATASENDINTERVAL);
    Serial.println(" seconds.");
  }
  else {
    Serial.print("Upload failed, next upload will be in ");
    Serial.print(DATASENDSHORTINTERVAL);
    Serial.println(" seconds.");
  }
}

//returns the lowest value of up to two sensors
float getLowestTemperature() {
  sensors.requestTemperatures();
  float tempC = sensors.getTempCByIndex(0);
    
  if (sensors.getDeviceCount() == 2)
    tempC = min(tempC, sensors.getTempCByIndex(1));

  return tempC;
}

//gathers and passes necessary data to be displayed on OLED
void mainScreen() {
  //for getDeviceCount() to be updated, begin() needs to be called
  sensors.begin();
  
  //signal strenght of wifi connection mapped to 0-4 to represent bars
  int strength = map(TRESP::getRSSIasPercentage(), 0, 100, 0, 4);
  
  sensors.requestTemperatures();
  //array containing the value of the two sensors
  float temperatures[2] {sensors.getTempCByIndex(0), sensors.getTempCByIndex(1)};
  display.mainScreen(strength, temperatures, sensors.getDeviceCount(), WIFI_getChipId());
}

//returns true the first "frame" the OLED_BUTTON is pushed
bool pressedSwitchButton() {
  bool pressed = false;
  int buttonState = digitalRead(OLED_BUTTON); //LOW if button is down
  static int lastButtonState = HIGH; //is only set to HIGH when first created 
  
  if(buttonState == LOW && lastButtonState == HIGH)
    pressed = true;
    
  lastButtonState = buttonState;
  return pressed;
}

//tells the display object to print a dot
void printDot() {
  display.printDot();
}

//returns true if enough seconds (intervalTimeS) has passed since lastUpdatedTime
//and updates lastUpdatedTime if true
bool timerDone(unsigned long *lastUpdatedTime, unsigned long intervalTimeS) {

  //if current time is less than lastUpdatedTime + given intervalTime
  if(millis() < *lastUpdatedTime + (intervalTimeS * 1000))
    return false; 
  Serial.print(millis());
  //set lastUpdatedTime to current time
  *lastUpdatedTime = millis();
  return true;
}
