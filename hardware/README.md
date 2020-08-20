# Hardware

The disaster.radio hardware is as open-as-possible. Currently, we have the schematic, BOM, and circuit board design for a development board that you can, theoretically, print yourself or order from a circuit board manufacturer of your choice. 

## Features
* ESP32-WROOM integrated WiFi + BLE microcontroller
* Dual LoRa modules
* Micro-SD card slot
* Micro-USB port for serial debugging
* Battery connection
* Battery charging module (currently using TPS6216)

## Developement
Our hardware was designed in KiCad, if you would like to edit the board deisgn or take a closer look at the schematics you can [download and install KiCad](https://kicad-pcb.org/download/) for your system.

After installing, open KiCad, choose `File > Open Project...`, navigate to where you cloned this repo, and select the file `hardware/board_esp32_v2/board_esp32_v2.pro`. You can now open and view the schematic and pcb design.
