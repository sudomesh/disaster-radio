
Disaster Radio is a work-in-progress long-range, low-bandwidth wireless disaster recovery mesh network powered by the sun.

* `firmware/` is the ESP8266 firmware
* `web/` is the web app served up by the ESP8266

The `web/` dir includes a simulator server that presents the same API as the ESP8266 to the client. This makes development of the web app possible without having the Disaster Radio hardware hooked up.

# Initial setup

```
./fetch_deps.sh # download dependencies
cp settings.mk.example settings.mk # create initial personal settings file
```

Then edit `settings.mk` to suit your needs.

# Hardware Connections  

NodeMCU/ESP8266 | SX1276   
----------------|--------
D3/GPIO0/OUT | RESET
D4/GPIO2/INT | DIO0   
D5/GPIO14/SCK | SCK  
D6/GPIO12/MISO | MISO   
D7/GPIO13/MOSI | MOSI   
D8/GPIO15/SS | NSS     
  
DIO0 sends an interrupt from the LoRa chip upon Tx/Rx Ready on the radio. It is run through GPIO2, which should be connected to the on-board LED of the ESP8266, thereby blinking a light upon transmit and recieve. 
  
# Building firmware

```
make
```

# Flashing firmware

```
make flash
```

# Building web app

See `web/README.md`

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

# Adding libraries

If you're including new libraries in the firmware then you wil need to add them to `LIBS =` in `config.mk`. 

Make sure to also include the approprate commands for fetching the new libraries in `fetch_deps.sh`.

# License

* `firmware/`: Dual licensed under both GPLv3 and AGPLv3
* `web/`: AGPLv3

