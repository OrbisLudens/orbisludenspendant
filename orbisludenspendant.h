#include <ESP8266HTTPClient.h>
#include <Adafruit_NeoPixel.h>
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <stdio.h>


// ------------------- CONFIG SECTION ---------------------

// Which pin on the Arduino is connected to the NeoPixels?
#define PIN            3
// How many NeoPixels are attached to the Arduino?
#define NUMPIXELS      12

// For a twelve LED Neopixel Ring we have an empty led after 2 Resonators. We mark them as "mod" (Mod slot)
const char* resonatorleds[] = { "N", "NE", "mod", "E", "SE", "mod", "S", "SW", "mod", "W", "NW", "mod" };

// Serial Console Speed
const unsigned long BAUD_RATE = 115200;

