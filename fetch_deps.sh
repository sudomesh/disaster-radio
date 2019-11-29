#!/bin/bash

set -e

BOARD=$1

if [ -d makeEspArduino ]; then rm -rf makeEspArduino; fi
if [ -d esp8266-arduino ]; then rm -rf esp8266-arduino; fi
if [ -d arduino-esp32 ]; then rm -rf arduino-esp32; fi
if [ -d libs ]; then rm -rf libs; fi

# get makeEspArduino
git clone https://github.com/plerup/makeEspArduino
cd makeEspArduino
git checkout tags/4.18.0
cp makeEspArduino.mk ../Makefile
cd ../

mkdir -p libs

if [ $BOARD == "esp8266" ]; then
    # install esp8266-arduino build environment
    git clone https://github.com/esp8266/Arduino.git esp8266-arduino
    cd esp8266-arduino/
    git checkout tags/2.4.0-rc1
    cd tools/
    python get.py
    cd ../../
    # install ESPAsyncTCP
    cd libs/
    git clone https://github.com/me-no-dev/ESPAsyncTCP
    cd ESPAsyncTCP/
    git checkout 991f855109d8038ed2cf0b5fb89792fcfa23549c
    cd ../../
fi

if [ $BOARD == "esp32" ]; then
    # install arduino-esp32 build environment
    git clone https://github.com/espressif/arduino-esp32 arduino-esp32
    cd arduino-esp32/
    git checkout tags/1.0.4
    cd tools/
    python get.py
    cd ../../
    # install ESPAsyncTCP
    cd libs/
    git clone https://github.com/me-no-dev/AsyncTCP
    # see https://community.platformio.org/t/compile-error-when-trying-to-use-the-asynctcp-library/8460
    cd AsyncTCP/
    git checkout c9df7cdda6302f509db9c09d9a514a45f9392105
    cd ../../
fi

cd libs/
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
git checkout 6893b0e914fd03e5efc4f0464d6bef6d715109e1
cd ../

git clone https://github.com/paidforby/AsyncSDServer
cd AsyncSDServer/
git checkout 13375c6be978cb34180378ecf4042a3a4a1f5eab
cd ../../


