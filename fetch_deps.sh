#!/bin/bash

set -e

rm -rf makeEspArduino arduino-esp32 libs Makefile

# get makeEspArduino
git clone https://github.com/plerup/makeEspArduino
cd makeEspArduino
git checkout tags/4.18.0
cp makeEspArduino.mk ../Makefile
cd ../

# https://dl.espressif.com/dl/package_esp32_index.json, http://arduino.esp8266.com/stable/package_esp8266com_index.json

# install arduino-esp32 build environment
git clone https://github.com/espressif/arduino-esp32 arduino-esp32
cd arduino-esp32/
git checkout tags/1.0.1 #??
cd tools/
python get.py
cd ../../

# install ESPAsyncTCP and ESPAsyncWebserver
mkdir -p libs
cd libs/

git clone https://github.com/me-no-dev/AsyncTCP
# see https://community.platformio.org/t/compile-error-when-trying-to-use-the-asynctcp-library/8460

cd AsyncTCP/
git checkout c9df7cdda6302f509db9c09d9a514a45f9392105
cd ../

git clone https://github.com/me-no-dev/ESPAsyncWebServer
cd ESPAsyncWebServer/
git checkout bed4146ec647fd1cdfd1d2485724369c92cab8e6
cd ../

git clone https://github.com/sandeepmistry/arduino-LoRa
cd arduino-LoRa/
git checkout tags/0.5.0
cd ../../
