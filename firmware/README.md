This firmware interfaces with the LoRa transciever and Javascript web apps.

# Setup and development
See the main [README](https://github.com/sudomesh/disaster-radio) to build the firmware

# Testing
Debugging can be done over serial using a tty interface, such as screen or minicom.
To test the web app:  
* connect to the SSID provided by the device, `disaster.radio <MAC address>` 
* navigate to http://disaster.local or http://192.168.4.1
* enter a nick
* send a message
* to toggle the local echo, type `$`
