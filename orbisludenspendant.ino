#include "orbisludenspendant.h"
#include "wifi.h"
#include "url.h"

// When we setup the NeoPixel library, we tell it how many pixels, and which pin to use to send signals.
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

void setup () {

  pinMode(LED_BUILTIN, OUTPUT);     // Initialize the LED_BUILTIN pin as an output
  digitalWrite(LED_BUILTIN, LOW);   // Turn the LED on (Note that LOW is the voltage level
  
  pixels.begin(); // This initializes the NeoPixel library.
  pixels.show(); // Initialize all pixels to 'off'



  Serial.begin(BAUD_RATE); // Set serial Speed
  Serial.println(); // newline to get rid of the serial garbage


  //clean FS, for testing
  // SPIFFS.format();


  // From https://github.com/tzapu/WiFiManager/tree/master/examples/AutoConnectWithFSParameters
  // read configuration from FS json
  Serial.println("mounting FS...");
  loadConfig();


  // Initialize and configure Wifimanager
  WiFiManager wifiManager;
  WiFiManagerParameter tecthulu_url("tecurl", "tecthulu url", tecurl, 100);
  WiFiManagerParameter wifi_offline_mode("offline_mode", "offline mode enabled (yes/no)", offline_mode, 3);
  WiFiManagerParameter wifi_offline_faction("offline_faction", "offline faction (e/r)", offline_faction, 3);
  wifiManager.setSaveConfigCallback(saveConfigCallback);
  wifiManager.setAPCallback(configModeCallback);
  wifiManager.addParameter(&tecthulu_url);
  wifiManager.addParameter(&wifi_offline_mode);
    wifiManager.addParameter(&wifi_offline_faction);

  // Double Reset == Enter Config AP
  if (drd.detectDoubleReset()) {
    Serial.println("Double Reset Detected");
    wifiManager.startConfigPortal(wm_ssid, wm_password);
  } else {
    Serial.println("No Double Reset Detected");
    wifiManager.autoConnect(wm_ssid, wm_password);
  }

  //read updated parameters
  strcpy(tecurl, tecthulu_url.getValue());
  strcpy(offline_mode, wifi_offline_mode.getValue());
  strcpy(offline_faction, wifi_offline_faction.getValue());

  if (String(offline_mode) == "y") {
    WiFi.mode(WIFI_OFF);
    Serial.println("offline mode enabled");
  } else {
    // try to connect or spawn AP if no saved values
    // We disable autoconnect so the Config portal is only activated by double reset
    // wifiManager.autoConnect(wm_ssid, wm_password);
    Serial.println("connected");
  }
   //save the custom parameters to FS
  if (shouldSaveConfig) {
    saveConfig();
  }
  digitalWrite(LED_BUILTIN, HIGH);  // Turn the LED off by making the voltage HIGH
  drd.stop();
}

void loop() {
  String pos;
  String portalres[8];
  int reshealth[8];
  int health;

  Serial.println("---");
  Serial.println(offline_mode);
  if (String(offline_mode) == "y" ) {
    // call the offline mode
    offlineMode();
    delay(100);    // loop through the offline cycle 
  } else if (WiFi.status() == WL_CONNECTED) { //Check WiFi connection status

    HTTPClient http;  //Declare an object of class HTTPClient

    http.begin(tecurl);  //Specify request destination
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
          color[faction]=health;
          setPixel(i, color[0], color[1], color[2]);

        } else {
          // light led with white or clear led if the resonator is not captured
          Serial.printf("%d: grey color!\n", i);
          setPixel(i, 2, 2, 2);
        }
      }
    }
    http.end();   //Close connection
    delay(5000);    //Send a request every 5 seconds
  }
}

//callback notifying us of the need to save config
void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}

// callback to enter config mode if reset button is pressed twice
void configModeCallback (WiFiManager *myWiFiManager) {
  Serial.println("Entered config mode");
  Serial.println(WiFi.softAPIP());

  // You could indicate on your screen or by an LED you are in config mode here
  // We don't want the next time the boar resets to be considered a double reset
  // so we remove the flag
  drd.stop();
}

// Loads the config from the eeprom
void loadConfig() {
  if (SPIFFS.begin()) {
    Serial.println("mounted file system");
    if (SPIFFS.exists("/config.json")) {
      //file exists, reading and loading
      Serial.println("reading config file");
      File configFile = SPIFFS.open("/config.json", "r");
      if (configFile) {
        Serial.println("opened config file");
        size_t size = configFile.size();
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);

        configFile.readBytes(buf.get(), size);
        DynamicJsonBuffer jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject(buf.get());
        json.printTo(Serial);
        if (json.success()) {
          Serial.println("\nparsed json");
          strcpy(tecurl, json["tecurl"]);
          strcpy(offline_mode, json["offline_mode"]);
          //strcpy(offline_faction, json["offline_faction"]);
        } else {
          Serial.println("failed to load json config");
        }
      }
    }
  } else {
    Serial.println("failed to mount FS");
  }
  //end read
}

// Writes the config to the eeprom
void saveConfig() {
 Serial.println("saving config");
  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.createObject();
  json["tecurl"] = tecurl;
  json["offline_mode"] = offline_mode;
  json["offline_faction"] = offline_faction;
  
  File configFile = SPIFFS.open("/config.json", "w");
  if (!configFile) {
    Serial.println("failed to open config file for writing");
  }

  json.printTo(Serial);
  json.printTo(configFile);
  configFile.close();
  //end save

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
  pixels.show();
}

// function for offline mode
void offlineMode() {
  if (String(offline_faction) == "e") {
    pulsecolor = pixels.Color(0, 255, 0);
  } else if (String(offline_faction) == "r") {
    pulsecolor = pixels.Color(0, 0, 255);
  } else {
    pulsecolor = pixels.Color(255, 255, 255);
  }
  cur_bright = 20;
  FactionPulse(80, false);
}

// FactionPulse for offline mode
// We change the overall brightness. This is not recommended but for this is the only
// task and we do not run into timing issues we can use it here.
// Normally we change the brightness within each Led with setPixelColor()
void FactionPulse(uint8_t wait, boolean fade) {
  uint16_t i,j;
  for(j=0; j<40; j++) {
    PulseBrightness(5, fade, 40);
    for(i=0; i<pixels.numPixels(); i++) {
      pixels.setPixelColor(i, pulsecolor);
    }
    pixels.show();
    delay(wait);
    if (fade && cur_bright == 0) {
      break;
    }
  }
}

// Pulse brightness
void PulseBrightness(uint8_t step, boolean fade, uint8_t maxb) {
  if (fade && (cur_bright+bright_diff) <= step ) {
    cur_bright=0;
  }
  else {
    if (cur_bright >= maxb) {
      bright_diff=-step;
    }
    else if (cur_bright <= 5) {
      bright_diff=step;
    }
    cur_bright=cur_bright+bright_diff;
  }
  pixels.setBrightness(cur_bright); //adjust brightness here
}



