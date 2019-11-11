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
git checkout tags/1.0.4
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
git checkout f13685ee97675be2ac9502d177d3024ebc49c1e0
cd ../

git clone https://github.com/sandeepmistry/arduino-LoRa
cd arduino-LoRa/
git checkout tags/0.6.1
cd ../

git clone https://github.com/sudomesh/LoRaLayer2
cd LoRaLayer2/
git checkout 0c9a9009c5964df5b0f07127cbeb4c4ce774345b
rm src/Layer1_Sim.cpp src/Layer1_Sim.h # delete unneeded Simulator library to avoid conflicts
cd ../../
