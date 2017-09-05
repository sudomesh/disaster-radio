
Disaster Radio is a work-in-progres long-range, low-bandwidth wireless disaster recovery mesh network powered by the sun.

* `esp8266/` is the ESP8266 firmware
* `web/` is the web app served up by the ESP8266

The `web/` dir includes a simulator server that presents the same API as the ESP8266 to the client. This makes development of the web app possible without having the Disaster Radio hardware hooked up.

You will need to install the ESP8266 addon for the Arduino IDE to upload the firmware. See [this guide](https://learn.sparkfun.com/tutorials/esp8266-thing-hookup-guide/installing-the-esp8266-arduino-addon).

To upload the html/css/javascript assets you will need to install the arduino-esp8266fs-plugin. Download the latest zip [from here](https://github.com/esp8266/arduino-esp8266fs-plugin/releases/tag/0.2.0) then extract and copy the `.jar` file into `<your_arduino_ide_dir>/tools/ESP8266FS/tool/`. Restart the Arduino IDE. The IDE will now have a new option `Tools -> ESP8266 Sketch Data Upload` which will generate a SPIFFS image from everything in the `data/` directory of the current project and upload it to the ESP8266.



