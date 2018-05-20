## Orbis Ludens Pendant

The technical part of the Orbis Ludens pendant, based on a ESP8266 Arduino, Neopixel Ring and the Tecthulu API

## Requirements:

### Hardware
* ESP8266 (WeMos D1 Mini or compatible model)
* Neopixel Ring 8 Leds (or compatible model)

### Software
* Arduino IDE (best to use, especially to download the board information and the libraries)
* CH340G Driver for USB to Serial Connection (https://wiki.wemos.cc/downloads). You need to install it (at least on MacOS to see the serial port of the Wemos D1 Mini)

### Arduino IDE
#### Libraries
To compile the sketch the following libraries need to be installed:

* ArduinoJson
* WifiManager (tzapu)
* Adafruit Neopixel
* DNSServer
* DoubleResetDetect (NOT DoubleResetDetect_or_)
* ESP8266HTTPClient
* ESP8266WebServer

#### Addition Board Information
We need to add the ESP8266 board definitions:

In the IDE add an additional Board Manager URL:

* Settings -> Additional Board Manager URL: http://arduino.esp8266.com/stable/package\_esp8266com\_index.json

Now choose the Board in the Board Manager:
* Tools -> Board -> Board Manager -> Search for "esp8266" -> Install
* Select the "Wemos D1 R2 & mini" board
* Select the port


## Configuration

* Copy url.h.example to url.h and configure the URL
* Copy wifi.h.example to wifi.h and configure the SSID and the password for the AP which will be spawned if a double reset is detected

### The configuration AP
The device can be configured by pressing the reset button 2 times within 3 seconds. After that an access point (ssid, password configured in wifi.h) spawns with a capative portal and redirects all traffic to the configuration URL. The main configuration can be found under "Wifi Configuration".

In the Wifi Configuration tab you can configure the AP the pendant should connect to. Furthermore the URL of the Tecthulu Device can be specified.

If you want to operate in offline mode you can set "offline_mode" to y or n (yes or no) and the color (r for resistance, e for enlightend, everything else will set the color to white). In the offline mode all leds will pulse. If you want to disable the offline mode you need to reenter the configuration gui and set offline_mode to n.

All data will be saved in the eeprom of the arduino. To delete this data you need to recompile the code and uncomment the line "SPIFFS.format()"


### Reconfigure
To manually spawn the AP double press the reset button. The device will boot up again and spawn the AP.

## What does it do
The Orbis Ludent Pendant is an pendant like object which interacts with the Orbis Ludent Portal on the Osiris Ephiphany Night Event on Schloss Kaltenberg (Ingress related event).
Whenever the portal resonators change the pendant show that change with 8 ultrabright leds. This works only near the Portal (technical: within the range of the configured Wifi AP).

The device can be set to an offline mode where it pulses in the chosen faction color.

### Example
Prototype with a 12 LED Ring (4 for mods, not used): https://www.youtube.com/watch?v=4cp5gsIQYR0
