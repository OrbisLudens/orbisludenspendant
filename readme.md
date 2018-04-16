## Orbis Ludens Pendant
based on a ESP8266 Arduino, Neopixel Ring and the Tecthulu API

## Requirements:

### Hardware
* ESP8266 (WeMos D1 Mini or compatible model)
* Neopixel Ring 8 Leds (or compatible model)

### Software
* Arduino IDE (best to use, especially to download the board informations and the libraries)
* CH340G Driver for USB to Serial Connection (https://wiki.wemos.cc/downloads). You need to install it (at least on MacOS to see the serial Port of the Wemos D1 Mini)

### Arduino IDE
#### Libraries
To compile the sketch the following libraries need to be installed:

* ArduinoJson
* WifiManager (tzapu)
* Adafruit Neopixel
* DNSServer
* DoubleResetDetector
* ESP8266HTTPClient
* ESP8266WebServer

#### Addition Board Information
We need to add the ESP8266 board definitions:

In the IDE add an additional Board Manager URL: 

* Settings -> Additional Board Manager URL: http://arduino.esp8266.com/stable/package\_esp8266com\_index.json

Now choose the Board in the Board Manager:
* Tools -> Board -> Board Manager -> Search for "esp8266" -> Install
* Select the "Wemos D1 R2 & mini" Board
* Select the Port

