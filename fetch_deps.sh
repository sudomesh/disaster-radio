#!/bin/bash

set -e

# get makeEspArduino
git clone https://github.com/plerup/makeEspArduino
cd makeEspArduino
git checkout tags/4.13.0
cp makeEspArduino.mk ../Makefile
cd ../

# install esp8266-arduino build environment
git clone https://github.com/esp8266/Arduino.git esp8266-arduino
cd esp8266-arduino/
git checkout tags/2.4.0-rc1
cd tools/
python get.py
cd ../../

# install ESPAsyncTCP and ESPAsyncWebserver
mkdir -p libs
cd libs/
git clone https://github.com/me-no-dev/ESPAsyncTCP
cd ESPAsyncTCP/
git checkout 991f855109d8038ed2cf0b5fb89792fcfa23549c
cd ../
git clone https://github.com/me-no-dev/ESPAsyncWebServer
cd ESPAsyncWebServer/
git checkout e6c432e56327c166bb71dbc0317654790dcbe3af
cd ../
git clone https://github.com/sandeepmistry/arduino-LoRa
cd arduino-LoRa/
git checkout 7f1ca664499e2406ac03ceca9b22c4406a921cda
cd ../../
