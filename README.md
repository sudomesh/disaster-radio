[disaster.radio](https://disaster.radio) is a work-in-progress long-range, low-bandwidth wireless disaster recovery mesh network powered by the sun.

* `firmware/` is the ESP8266 firmware
* `web/` is the web app served up by the ESP8266
* `hardware/` is the kicad board layout and schematic
* `enclosure/` are 3D models of the enclosure(s)

The `web/` dir includes a simulator server that presents the same API as the ESP8266 to the client. This makes development of the web app possible without having the Disaster Radio hardware hooked up.

# Layout and Flow
The general layout and flow of hardware, firmware, and software can be seen below:
![alt text](https://raw.githubusercontent.com/sudomesh/disaster-radio/master/diagram.png "disaster flow")

# Hardware Connections  

NodeMCU/ESP8266 | LoRa Transceiver | SD Card |
----------------|--------|---------|
D1/GPIO5/OUT | RESET | _NC_ 
D2/GPIO4/INT | DIO0 | _NC_ 
D4/GPIO2/CS | _NC_ | SS 
D5/GPIO14/SCK | SCK | SCK 
D6/GPIO12/MISO | MISO | MISO  
D7/GPIO13/MOSI | MOSI | MOSI  
D8/GPIO15/CS | NSS | _NC_   
  
DIO0 sends an interrupt from the LoRa chip upon Tx/Rx Ready on the radio. 
Chip selects can be used to explicitly switch between the LoRa tranceiver and the SD card; however, the SD card should be enabled by default, as the LoRa interrupt appears to handle enabling the tranceiver (note: this still needs to be thoroughly tested, what happens if you receive an interrupt while loading from the SD?).

# Websocket

The disaster.radio firmware opens up a websocket using [the ESPAsyncWebServer library](https://github.com/me-no-dev/ESPAsyncWebServer). Through this, client-side javascript can transmit and receive messages over the LoRa tranceiver. If you'd like to build an application for disaster.radio, you will need to write a websocket client that can send and receive messages in the proper format to the websocket server running on the ESP8266. Currently, the firmware expects websocket messages in the following format,   
`<msgID><msgType>|<msg>`  
where,
* `<msgID>` is a two-byte binary unsigned integer representing an abitrary sequence number, this is sent back to the websocket client with an `!` appended to act as an acknowledgment and could be used for error-checking,  
* `<msgType>` is a single binary utf8 encoded character representing the application for which the message is intended, such 'c' for chat, 'm' for maps, or 'e' for events  
* `<msg>` is a binary utf8 encoded string of characters limited to 252 bytes, this can be treated as the body of the message and may be used to handle client-side concerns, such as intended recipient or requested map tile.    

An example messge may appear as follows,
`0100c|<noffle>@juul did you feel that earthquake!`
or
`0400m|<juul>{request:{tile:[[2.115, -59.28],[2.345,-59.05]]}`

An example chat app can be found in the [web directory](https://github.com/sudomesh/disaster-radio/tree/master/web).

# Initial setup

```
./fetch_deps.sh esp8266 # download dependencies if using WeMos D1 with disaster radio hat
cp config.mk.esp8266 config.mk
cp settings.mk.example settings.mk # create initial personal settings file
sudo pip install esptool
```
OR
```
./fetch_deps.sh esp32 # if using ESP32 TTGO board
cp config.mk.esp32 config.mk
cp settings.mk.example settings.mk # create initial personal settings file
sudo pip install esptool
```

Then edit `settings.mk` to suit your needs. If you are flashing a board based on the wemos d1 mini (with built-in usb) then you will probably not have to edit anything in `settings.mk` but make sure `UPLOAD_PORT` is set to the correct device which may vary depending on your operating system and which other devices you have connected.
  
# Building firmware

```
make
```

# Flashing firmware

## For the wemos d1 mini dev board

Connect your computer the board using a usb cable,

In `settings.mk` make sure to uncomment the lines for the WeMos D1 Mini and comment the lines for the solar module. You'll have to do the same for the lines in `firmware/firmware.ino` defining `csPin`, `resetPin`, and `irqPin`.

Then run:

```
make flash
```

## For the full solar + li-ion board

This board does not have usb so flashing is slightly more complicated. The pin header immediately to the left of the ESP-07 board has two labels: 

* UART at the top
* PGM on the bottom

UART means normal operation and PGM means programming/flashing mode. To switch between these modes, connect a jumper between the middle pin and either the top or bottom pin. If the jumper is in the bottom position then you are in flashing mode.

You will need a USB to serial 3.3v adapter. DO NOT USE A 5v SERIAL CABLE!

Connect the wires like so between the USB-to-serial adapter and the disaster.radio board:

```
USB-to-serial |  disaster.radio
------------------------------
         GND <-> GND
          TX <-> RX
          RX <-> TX
         VCC <-> 3V3
```

It is possible that the power supplied by the 3.3 V output of your serial adapter is not enough to power the disaster.radio board. If this is the case you will have to supply 3.3 V some other way. The simplest is to use a 3.3 V voltage regulator such as the LD1117V33 connected to the 5 V output from the USB port of your computer. The voltage regulator will have three pins:

* GND
* Vout
* Vin

On the USB-to-serial adapter you should be able to find a pin or connection point that has the 5 V output from your computer. Connect this to Vout on the voltage regulator, then connect GND to GND (you can use the outer part of the USB plug if nothing else is available) and Vout will be the output that you connect to the 3V3 pin on the disaster.radio board. It's a good idea to put a ~100 uF capacitor between GND and Vout.

In `settings.mk` make sure to comment the lines for the WeMos D1 Mini and uncomment the lines for the solar module. You'll have to do the same for the lines in `firmware/firmware.ino` defining `csPin`, `resetPin`, and `irqPin`.

Now you can use `make flash` and `make flash_fs` normally, however note that you will have to manually reset the board after each flashing operation.

After running `make flash_fs` you may get an error that looks like this:

```
A fatal error occurred: Timed out waiting for packet header
make: *** [flash_fs] Error 2
```

If you run into a problem where the `make flash_fs` seems to complete but the files aren't readable by the firmware, try this:

```
make erase_flash
make flash_fs
make flash
```

You can safely ignore this error.

Remember to power the board off and moving the jumper back to the top position once you're done flashing. The simplest way to power off the board is to unplug the usb cable from your computer. If this gets annoying you can connect/solder a switch to flip between the two modes and another switch to toggle the power.


# Building web app

See [web/README.md](https://github.com/sudomesh/disaster-radio/tree/master/web)

# Building and uploading SPIFFS image

To build:

```
make fs
```

To build and upload:

```
make flash_fs
```

If building doesn't work then try this first:

```
touch web/static/*
```

If using an SD card model, copy the contents of `web/static/` to the root of a fat32 formatted SD card.

# Testing the firmware and web app  
See [firmware/README.md](https://github.com/sudomesh/disaster-radio/tree/master/firmware)

# Adding libraries

If you're including new libraries in the firmware then you wil need to add them to `LIBS =` in `config.mk`. 

Make sure to also include the approprate commands for fetching the new libraries in `fetch_deps.sh`.

# License

* `firmware/`: Dual licensed under both GPLv3 and AGPLv3
* `web/`: AGPLv3

