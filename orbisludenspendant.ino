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
        } else {
          Serial.println("failed to load json config");
        }
      }
    }
  } else {
    Serial.println("failed to mount FS");
  }
  //end read

  // Initialize and configure Wifimanager
  WiFiManager wifiManager;
  WiFiManagerParameter tecthulu_url("tecurl", "tecthulu url", tecurl, 100);
  WiFiManagerParameter wifi_offline_mode("offline_mode", "offline mode enabled (yes/no)", offline_mode, 3);
  wifiManager.setSaveConfigCallback(saveConfigCallback);
  wifiManager.setAPCallback(configModeCallback);
  wifiManager.addParameter(&tecthulu_url);
  wifiManager.addParameter(&wifi_offline_mode);

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
    Serial.println("saving config");
    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();
    json["tecurl"] = tecurl;
    json["offline_mode"] = offline_mode;
    
    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile) {
      Serial.println("failed to open config file for writing");
    }

    json.printTo(Serial);
    json.printTo(configFile);
    configFile.close();
    //end save
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
    delay(1000);    // loop through the offline cycle 
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


void offlineMode() {
  Serial.println("We have entered the offline mode!");
}




