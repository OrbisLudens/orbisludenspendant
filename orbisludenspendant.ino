#include "orbisludenspendant.h"
#include "wifi.h"
#include "url.h"

// When we setup the NeoPixel library, we tell it how many pixels, and which pin to use to send signals.
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

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
  int reshealth[8];
  int health;

  Serial.println("---");
  if (WiFi.status() == WL_CONNECTED) { //Check WiFi connection status

    HTTPClient http;  //Declare an object of class HTTPClient

    http.begin(url);  //Specify request destination
    int httpCode = http.GET();  //Send the request

    if (httpCode > 0) { //Check the returning code

      String payload = http.getString();   //Get the request response payload
      //Serial.println(payload);             //Print the response payload
      StaticJsonBuffer<2000> JSONBuffer; // 2000 byte is the max length of a message
      JsonObject& tec_answer = JSONBuffer.parseObject(payload);

      int faction = (int) tec_answer["status"]["controllingFaction"]; // Controlling faction. 0=none, 1=ENL, 2=RES
      Serial.println(faction);
      Serial.println(tec_answer["status"]["resonators"].as<String>());
      int deployedresos = tec_answer["status"]["resonators"].size(); // Number of deployed resonators
      Serial.printf("Deployed Resos: %d\n", deployedresos);
      // Iterate over the json resonator subarray and get the health and the position
      for (int i = 0; i < deployedresos; i++) {
        reshealth[i] = (int) tec_answer["status"]["resonators"][i]["health"];
        portalres[i] = tec_answer["status"]["resonators"][i]["position"].as<String>();
      }
      // Now iterate over the pixels and see if we have a resonator to show
      for (int i = 0; i < NUMPIXELS; i++) {
        Serial.printf("-------        Checking >> %s <<\n", resonatorleds[i]);
        int pos = isvalueinarray((String)resonatorleds[i], portalres, 8);
        if (pos != -1 ) {
          // light led with faction color
          health = reshealth[pos];
          Serial.printf("%d: faction color (%d health)\n", i, health);
          int color[3] = {0, 0, 0};
          color[faction]=health+20;
          setPixel(i, color[0], color[1], color[2]);

        } else {
          // light led with white or clear led if the resonator is not captured
          Serial.printf("%d: grey color!\n", i);
          setPixel(i, 20, 20, 20);
        }
      }
    }

    http.end();   //Close connection

  }

  delay(5000);    //Send a request every 5 seconds

}

// Find the LED according to the position
int findLedPosition(String pos) {
  for (int i = 0; i < sizeof(resonatorleds)-1; i++) {
    if ((String) resonatorleds[i] == pos) {
      return i;
    }
  }
  return -1;
}

// Search a value in an array and return the position
int isvalueinarray(String val, String arr[], int size){
    int i;
    for (i = 0; i < size; i++) {
        if (val.equals(arr[i])) {
            return i;
        }
    }
    return -1;
}

// Light up that pixel!
void setPixel(int pos, int r, int g, int b) {
  Serial.printf("Set Pixel %d to (%d, %d, %d)\n", pos, r, g, b);
  pixels.setPixelColor(pos, r, g, b);
}




