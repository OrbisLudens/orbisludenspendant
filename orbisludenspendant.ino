#include <Adafruit_NeoPixel.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <stdbool.h>


// ------------------- CONFIG SECTION ---------------------

// Which pin on the Arduino is connected to the NeoPixels?
#define PIN            3
// How many NeoPixels are attached to the Arduino?
#define NUMPIXELS      12

// Serial Console Speed
const unsigned long BAUD_RATE = 115200; 
// WIFI SSID
const char* ssid = "xxx";
// WIFI Password 
const char* password = "xxx";
// URL to fetch
const char* url = "http://5.45.98.140:8080/module/status/json";

// ---------------- CONFIG SECTION END -------------------


// When we setup the NeoPixel library, we tell it how many pixels, and which pin to use to send signals.
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

// For a twelve LED Neopixel Ring we have an empty led after 2 Resonators
const char* resonatorleds[] = { "N", "NE", "", "E", "SE", "", "S", "SW", "", "W", "NW", "" };
// Array with faction colors according to tecthulu module definition
// 0 = grey(white), 1 = green, 2 = blue
const uint32_t factioncolors[] = { pixels.Color(0, 0, 0), pixels.Color(0, 255, 0), pixels.Color(0, 0, 255) };



void setup () {

  pixels.begin(); // This initializes the NeoPixel library.
  pixels.show(); // Initialize all pixels to 'off'
  
  Serial.begin(BAUD_RATE); // Set serial Speed
  Serial.println(); // newline to get rid of the serial garbage
  
  WiFi.begin(ssid, password);
 
  while (WiFi.status() != WL_CONNECTED) {
 
    delay(1000);
    Serial.print("Connecting..");
 
  }
}
 
void loop() {
  String pos;
  String portalres[8];
 
  if (WiFi.status() == WL_CONNECTED) { //Check WiFi connection status
 
    HTTPClient http;  //Declare an object of class HTTPClient
 
    http.begin("http://5.45.98.140:8080/module/status/json");  //Specify request destination
    int httpCode = http.GET();                                                                  //Send the request
 
    if (httpCode > 0) { //Check the returning code
 
      String payload = http.getString();   //Get the request response payload
      Serial.println(payload);             //Print the response payload
      StaticJsonBuffer<2000> JSONBuffer;
      JsonObject& tec_answer = JSONBuffer.parseObject(payload);
      
      int faction = (int) tec_answer["status"]["controllingFaction"];
      Serial.println(faction);
      Serial.println(tec_answer["status"]["resonators"].as<String>());
      int deployedresos = tec_answer["status"]["resonators"].size();
      for (int i = 0; i < deployedresos; i++) {
        pos = tec_answer["status"]["resonators"][i]["position"].as<String>();
        portalres[i] = pos;
        //Serial.println(pos);
        //int posnr = findLedPosition(pos);
        //Serial.println(posnr);
      }
      for (int i = 0; i < NUMPIXELS-1; i++) {
        if (isvalueinarray(resonatorleds[i], portalres, 8)) {
          // light led with faction color
        } else {
          // light led with white or clear led
        }
      }
    }
 
    http.end();   //Close connection
 
  }
 
  delay(5000);    //Send a request every 5 seconds
 
}

// Find the LED according to the position
int findLedPosition(String pos) {
  for (int i = 0; i < sizeof(resonatorleds) -1; i++) {
    if ((String) resonatorleds[i] == pos) {
      return i;
    }
  }
  return -1;
}


bool isvalueinarray(int val, int *arr, int size){
    int i;
    for (i=0; i < size; i++) {
        if (arr[i] == val)
            return true;
    }
    return false;
}







