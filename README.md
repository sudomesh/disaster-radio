[disaster.radio](https://disaster.radio) is a work-in-progress long-range, low-bandwidth wireless disaster recovery mesh network powered by the sun.  

[![Build Status](https://travis-ci.org/sudomesh/disaster-radio.svg?branch=master)](https://travis-ci.org/sudomesh/disaster-radio)  

* [firmware/](./firmware) contains the main example code for ESP32 dev boards
* [src/](./src) contains the disaster radio library code used by the firmware and simulator
* [web/](./web) is the demo chat app served up by the disaster-radio
* [hardware/](./hardware) is the kicad board layout and schematic
* [enclosure/](./enclosure) are 3D models of the enclosure(s)
* [android app](https://github.com/beegee-tokyo/disaster-radio-android) source code for [DisasterRadio BLE Chatbox](https://play.google.com/store/apps/details?id=tk.giesecke.disaster_radio&hl=en_US) Android Application
* [mapping web app](https://github.com/sudomesh/disaster-radio-map) source code for mapping web-based appilcation

## Table of contents
- [Getting Started](#getting-started)
- [Layout and Flow](#layout-and-flow)
- [Hardware Connections](#hardware-connections)
- [Initial Setup with PlatformIO](#initial-setup-with-platformio)
    - [Building firmware](#building-firmware)
    - [Flashing firmware](#flashing-firmware)
        - [For dev boards](#for-dev-boards)
- [Building Web App](#building-web-app)
    - [Building and uploading SPIFFS image](#building-and-uploading-SPIFFS-image)
- [Testing Firmware](#testing-firmware) 
- [Adding Libraries](#adding-libraries)
- [Creating Binary for Release](#creating-binary-for-release)
- [License](#license)

# Getting Started
The quickest way to get started with disaster.radio is to buy a <a href="https://www.aliexpress.com/item/4000396836096.html" target="_blank">preinstalled device</a> or
* Download the [latest release](https://github.com/sudomesh/disaster-radio/releases)
* Unzip the `disaster-radio-0.X.X.zip` file
* Follow the included instructions for flashing it to your device

If you would rather test the latest, cutting-edge developments, you can compile the firmware yourself by cloning this repo and following the [initial setup](#initial-setup-with-platformio) instructions.

# Layout and Flow
The Disaster Radio firmware is made up of a single "server" and a number of "clients", as well as "middleware" that sits between the clients and server. These are a"server" and "clients" in the logical, software sense, not a literal, networking sense.
 
Here are the modules currently implemented with descriptions of their purpose:
- DisasterServer:  
    - DisasterRadio: the main "server" that takes messages from clients and sends them to other clients

- DisasterClient:  
    - LoRaClient: interfaces with LoRaLayer2  
    - WebSocketClient: WebSocket connections from the web app  
    - StreamClient: for Arduino Stream, currently used for serial console  
    - TCPClient: for a telnet-like server  
    - HistoryRecord: records messages to SD card (or bounded queue in memory)  
    - OLEDClient: displays messages on the screen (mostly for debugging purposes, but eventually I'd like to use this for a mobile Disaster Radio terminal of some kind)   
    - GPSClient: proof-of-concept, interfaces with a serial GPS module to beacon your current location periodically  

- DisasterMiddleware:  
    - WelcomeMessage: (very simple, in main.cpp) shows a welcome message to clients when they connect
    - HistoryReplay: shows history to clients when they connect
    - Console: implements a simple console with chat (similar to the web app) plus some /commands (I went with a more IRC-like syntax but you could easily implement a getopt version)
    
- DisasterHistory:
    - HistorySD: records history to SD card
    - HistoryMemory: records history to a bounded queue in memory (default limit 10 messages)

The server and clients communicate with each other using "datagrams" that include a destination address, a message type, and a message. Read more about the disaster radio networking stack on our wiki, https://github.com/sudomesh/disaster-radio/wiki/Layered-Model.

# Hardware Connections  

For supported development boards see the [Devices & Hardware](https://github.com/sudomesh/disaster-radio/wiki/Devices--&-Hardware) wiki page.  

The pinouts for supported boards can be found in [firmware/esp32/config.h](https://github.com/sudomesh/disaster-radio/blob/master/firmware/esp32/config.h).  

DIO0 sends an interrupt from the LoRa transceiver upon Tx/Rx Ready on the radio.   
Chip selects can be used to explicitly switch between the LoRa tranceiver and the SD card; however, the SD card should be enabled by default, as the LoRa interrupt appears to handle enabling the transceiver (note: this still needs to be thoroughly tested, what happens if you receive an interrupt while loading from the SD?).  

# Initial Setup with PlatformIO

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

Connect your computer to the board using a usb cable,

In `platformio.ini` make sure `upload_port`, `upload_speed`, etc are correct for your device.

Then run:

```
pio run -t upload
```
 
By default, PlatformIO builds and flashes firmware for the LILY's ESP32 TTGO V2 dev board. If you would like to build for another supported board, select the corresponding build environment. For example to build and flash the firmware and file system for the ESP32 TTG0 V1 board, use the following, 

```
pio run -e ttgo-lora32-v1 -t upload -t uploadfs

```

## Developing firmware

Some useful hints for developing this firmware using platformio are included here,
* It may be a good idea to delete the libdeps folder prior to rebuilding, as old, out-dated libraries could case conflits. To do this, `rm -rf .pio/libdeps`.
* If you would like to make changes to a specific library, such as LoRaLayer2, you can clone the library into the `firmware/lib` folder that is created after running `pio run` and then comment it out or remove it from the `lib_deps` list in the `platformio.ini` file.

# Building Web App

First, build the web app using npm, instructions for can be found in [web/README.md](https://github.com/sudomesh/disaster-radio/tree/master/web#setup).

If you have trouble building the web app with npm, you can download a pre-built version of web app as part of the [latest release](https://github.com/sudomesh/disaster-radio/releases). Then, copy the files in `web/static/` from the zipped release to the `web/static/` directory in the cloned repo.


## Building and uploading SPIFFS image

Next, build the binary for the SPIFFS and upload it to your board.

To only build:

```
pio run -t buildfs
```

To build and upload:

```
pio run -t uploadfs
```

If using an SD card, copy the contents of `web/static/` to the root of a fat32 formatted SD card, then insert the SD card.

# Testing Firmware  
Once the firmware and SPIFFS image has been successfully flashed, you can test it by logging into the `disaster.radio <node-address>` wireless network and navigating to http://192.168.4.1 in a browser to access the demo chat app.  

You can also try connecting to the device via serial or telnet with commands like,  
```
pio device monitor
```
or
```
telnet 192.168.4.1
```

After pressing enter once, you should be greeted with a banner and presented with a prompt to enter a nick or send an anonymous message,
```
     ___              __                            ___    
 ___/ (_)__ ___ ____ / /____ ____      _______ ____/ (_)__ 
/ _  / (_-</ _ `(_-</ __/ -_) __/ _   / __/ _ `/ _  / / _ \
\_,_/_/___/\_,_/___/\__/\__/_/   (_) /_/  \_,_/\_,_/_/\___/
v1.0.0-rc.2
LoRa transceiver connected
Local address of your node is 1d69bd4c
Type '/join NICKNAME' to join the chat, or '/help' for more commands.
< > 
```

See [firmware/README.md](https://github.com/sudomesh/disaster-radio/tree/master/firmware) for more debugging information.

# Adding Libraries

If you're including new libraries in the firmware, for PlatformIO, you wil need to add them to `platformio.ini` under `lib_deps`.

# Creating Binary for Release

A full binary image can be created by reading the flash contents of a ESP32 that is flashed with the latest release of the firmware.
To do this, run the following command,
```
esptool.py -p /dev/ttyUSB0 -b 921600 read_flash 0 0x400000 esp_flash.bin
```

This can then be flashed to a new device like so,
```
esptool.py -p /dev/ttyUSB0 --baud 460800 write_flash 0x00000 esp_flash.bin
```

# License

* `firmware/`: Dual licensed under both GPLv3 and AGPLv3
* `web/`: AGPLv3

