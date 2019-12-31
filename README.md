[disaster.radio](https://disaster.radio) is a work-in-progress long-range, low-bandwidth wireless disaster recovery mesh network powered by the sun.

* [firmware/](./firmware) contains the main firmware for ESP32 and ESP8266
* [web/](./web) is the demo chat app served up by the disaster-radio
* [hardware/](./hardware) is the kicad board layout and schematic
* [enclosure/](./enclosure) are 3D models of the enclosure(s)

## Table of contents
- [Getting Started](#getting-started)
- [Layout and Flow](#layout-and-flow)
- [Hardware Connections](#hardware-connections)
- [Websocket](#websocket)
- [Initial Setup](#initial-setup)
    - [Building firmware](#building-firmware)
    - [Flashing firmware](#flashing-firmware)
        - [For dev boards](#for-dev-boards)
        - [For the full solar + li-ion board](#for-the-full-solar--li-ion-board)
- [Building Web App](#building-web-app)
    - [Building and uploading SPIFFS image](#building-and-uploading-SPIFFS-image)
- [Testing Firmware](#testing-firmware) 
- [Adding Libraries](#adding-libraries)
- [License](#license)

# Getting Started
The quickest way to get started with disaster.radio is to,
* Download the [latest release](https://github.com/sudomesh/disaster-radio/releases)
* Unzip the `disaster-radio-0.X.X.zip` file
* Follow the included instructions for flashing it to your device

If you would rather test the latest, cutting-edge developments, you can compile the firmware yourself by cloning this repo and following the [initial setup](#initial-setup) instructions.

# Layout and Flow
The general layout and flow of hardware, firmware, and software can be seen below:
![alt text](https://raw.githubusercontent.com/sudomesh/disaster-radio/master/diagram.png "disaster flow")

# Hardware Connections  

NodeMCU/ESP8266 | ESP32 | LoRa Transceiver | SD Card |
----------------|-------|--------|---------|
D1/GPIO5/OUT | GPIO23  |RESET | _NC_ 
D2/GPIO4/INT | GPIO26 | DIO0 | _NC_ 
D4/GPIO2/CS | GPIO2 | _NC_ | SS 
D5/GPIO14/SCK | ??? | SCK | SCK 
D6/GPIO12/MISO | ??? | MISO | MISO  
D7/GPIO13/MOSI | ??? | MOSI | MOSI  
D8/GPIO15/CS | GPIO18 |NSS | _NC_   
  
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

# Initial Setup

```
pip install -U platformio
```

Edit `platformio.ini` to suit your needs. If you are flashing a ESP32 LILY TTGO board then you will probably not have to edit anything in `platformio.ini` but make sure `upload_port` is set to the correct device which may vary depending on your operating system and which other devices you have connected.

## Building firmware

To test that the firmware is compiling correctly without flashing it to a device, run
```
pio run
```

This will compile all libraries and main firmware files. The resulting binary is stored as `.pio/build/ttgo-lora32-v1/firmware.bin`

## Flashing firmware

### For dev boards

Connect your computer the board using a usb cable,

In `platformio.ini` make sure `upload_port`, `upload_speed`, etc are correct for your device.

Then run:

```
pio run -t upload -t uploadfs
```

### For the full solar + li-ion board

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

In `platformio.ini` make sure to comment the lines for the WeMos D1 Mini and uncomment the lines for the solar module. You'll have to do the same for the lines in `firmware/firmware.ino` defining `csPin`, `resetPin`, and `irqPin`.

Now you can use `pio run -t upload` and `pio run -t uploadfs` normally, however note that you will have to manually reset the board after each flashing operation.

After running `pio run -t uploadfs` you may get an error that looks like this:

```
A fatal error occurred: Timed out waiting for packet header
```

If you run into a problem where the `pio run -t uploadfs` seems to complete but the files aren't readable by the firmware, try this:

```
pio run -t erase -t uploadfs -t upload
```

Remember to power the board off and moving the jumper back to the top position once you're done flashing. The simplest way to power off the board is to unplug the usb cable from your computer. If this gets annoying you can connect/solder a switch to flip between the two modes and another switch to toggle the power.


# Building Web App

The `web/` dir includes a simulator server that presents the same API as the ESP8266 to the client. This makes development of the web app possible without having the Disaster Radio hardware hooked up.

See [web/README.md](https://github.com/sudomesh/disaster-radio/tree/master/web)

## Building and uploading SPIFFS image

To build:

```
pio run -t buildfs
```

To build and upload:

```
pio run -t uploads
```

If building doesn't work then try this first:

```
touch web/static/*
```

If using an SD card model, copy the contents of `web/static/` to the root of a fat32 formatted SD card.

# Testing Firmware  
Once the firmware and SPIFFS image has been successfully flashed, you can test it by logging into the `disaster.radio <mac-address>` wireless network and navigating to http://192.168.4.1 in a browser to access the demo chat app.

You can also perform some tests by logging into the disaster.radio's console interface. If flashed correctly, you should be able to log into the device using a serial tool, such as screen or minicom with a baud rate of 115200. You will be greeted by a banner that looks like this,
```
     ___              __                            ___    
 ___/ (_)__ ___ ____ / /____ ____      _______ ____/ (_)__ 
/ _  / (_-</ _ `(_-</ __/ -_) __/ _   / __/ _ `/ _  / / _ \
\_,_/_/___/\_,_/___/\__/\__/_/   (_) /_/  \_,_/\_,_/_/\___/
v0.1.0
LoRa tranceiver connected
Local address of your node is d8a01d69bd4c

```
You can then run commands to confirm the device is operating correctly. To print the node's local address, use `lr -a`.
```
/# lr -a
1: lora1:
    address: d8a01d69bd4c
```
  
To print the current routing table of the node, use `lr -r`.

```
/# lr -r

Routing Table: total routes 1
1 hops from d8a01d69d448 via d8a01d69d448 metric 204 
```
  
To transmit a message over the lora transceiver, use `tx`.
```
/# tx -m heythere -d d8a01d69d448 -t c
TXing: heythere to d8a01d69d448 as type 'c'
```

See [firmware/README.md](https://github.com/sudomesh/disaster-radio/tree/master/firmware) for more debugging information.

# Adding Libraries

If you're including new libraries in the firmware then you wil need to add them to `platformio.ini` under `lib_deps`.

# License

* `firmware/`: Dual licensed under both GPLv3 and AGPLv3
* `web/`: AGPLv3

