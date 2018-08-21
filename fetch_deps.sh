#!/bin/bash

set -e

# get makeEspArduino
git clone https://github.com/plerup/makeEspArduino
cd makeEspArduino
git checkout tags/4.14.1
cp makeEspArduino.mk ../Makefile
cd ../

# install arduino-esp32 build environment
git clone https://github.com/espressif/arduino-esp32 arduino-esp32
cd arduino-esp32/
git checkout tags/1.0.0
cd tools/
python get.py
cd ../../

# install ESPAsyncTCP and ESPAsyncWebserver
mkdir -p libs
cd libs/
git clone https://github.com/me-no-dev/AsyncTCP
cd AsyncTCP/
git checkout 5453ec2e3fdd9beb92b8423878008b57be7805e6
cd ../
git clone https://github.com/me-no-dev/ESPAsyncWebServer
cd ESPAsyncWebServer/
git checkout bed4146ec647fd1cdfd1d2485724369c92cab8e6
cd ../
git clone https://github.com/sandeepmistry/arduino-LoRa
cd arduino-LoRa/
git checkout tags/0.5.0
cd ../../
