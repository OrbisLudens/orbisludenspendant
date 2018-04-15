// Access EEPROM as filesystem
#include <FS.h> 

// Neopixel ring library
#include <Adafruit_NeoPixel.h>

// Wifi connection Library
#include <ESP8266WiFi.h>

// Webserver library to spawn the Wifimanager Webserver (config portal)
#include <ESP8266WebServer.h>

// http client to GET an URL
#include <ESP8266HTTPClient.h>

// JSON parsing library
#include <ArduinoJson.h>

// default stdio lib
#include <stdio.h>

// Wifimanager to handle Wifi and custom configuration
#include <WiFiManager.h>

// DNS Library to redirect all HTTP calls on the config AP to the config url
#include <DNSServer.h>

// Recognizes double press of reset button to reenter config mode
#include <DoubleResetDetector.h>


//flag for saving data
bool shouldSaveConfig = false;

// Which pin on the Arduino is connected to the NeoPixels?
#define PIN            14
// How many NeoPixels are attached to the Arduino?
#define NUMPIXELS      12

// For a twelve LED Neopixel Ring we have an empty led after 2 Resonators. We mark them as "mod" (Mod slot)
const char* resonatorleds[] = { "N", "NE", "mod", "E", "SE", "mod", "S", "SW", "mod", "W", "NW", "mod" };

// Serial Console Speed
const unsigned long BAUD_RATE = 115200;

// Number of seconds after reset during which a
// subseqent reset will be considered a double reset.
// This sketch uses drd.stop() rather than relying on the timeout
#define DRD_TIMEOUT 10

// RTC Memory Address for the DoubleResetDetector to use
#define DRD_ADDRESS 0

DoubleResetDetector drd(DRD_TIMEOUT, DRD_ADDRESS);

