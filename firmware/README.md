This firmware interfaces with the LoRa transciever and Javascript web apps.

# Setup and development
See the main [README](https://github.com/sudomesh/disaster-radio) to build the firmware

## Connect to existing WiFi

In WiFi mode, the firmware by default creates it's own SSID. If you want to connect to your own Wifi instead, 
copy the file `secrets.H` to `secrets.h` and modify the file accordingly

    /* Place SSIDs, Passwords, Hashes, etc. in this file, it will not be tracked by git */
    
    // Uncomment to enable the Wifi client
    // #define WIFI_SSID "WIFI_SSID"
    // #define WIFI_PASSWORD "WIFI_PASSWORD"

Then follow the build instruction in the main [README](https://github.com/sudomesh/disaster-radio)

# Testing
Debugging can be done over serial using a tty interface, such as screen or minicom.
To test the web app:  
* connect to the SSID provided by the device, `disaster.radio <MAC address>` 
* navigate to http://disaster.local or http://192.168.4.1
* enter a nick
* send a message
* to toggle the local echo, type `$`
