
This is the web app served up by the Disaster Radio firmware + the simulator web server.

# Dependencies

This web app is intended to be built using Node.JS version 7.10.1. It is recommended that a tool such as [nvm](https://github.com/nvm-sh/nvm#installing-and-updating) is used to manage your Node and NPM versions.   
If you have `nvm` installed, you can install and activate the correct version of Node.JS like so,
```
nvm install 7.10.1
nvm use 7.10.1
```  

# Setup

```
npm install
cp settings.js.example settings.js
```

# Building and running

The first time you build you need to do:

```
npm run build-css
```

```
npm run build # build the js and css
npm start # run the simulator server
```

Then in a browser open http://localhost:8000/

# Developing

```
npm run dev # starts simulator server and auto-builder
```

# Websocket

The disaster.radio firmware opens up a websocket using [the ESPAsyncWebServer library](https://github.com/me-no-dev/ESPAsyncWebServer). Through this, client-side javascript can transmit and receive messages over the LoRa tranceiver. If you'd like to build an application for disaster.radio, you could write a websocket client that sends and receives messages in the same format. Currently, the firmware expects websocket messages in the following format:   
`<msgID><msgType>|<msg>`  
where,
* `<msgID>` is a two-byte binary unsigned integer representing an abitrary sequence number, this is sent back to the websocket client with an `!` appended to act as an acknowledgment and could be used for error-checking,  
* `<msgType>` is a single binary utf8 encoded character representing the application for which the message is intended, such 'c' for chat, 'm' for maps, or 'e' for events  
* `<msg>` is a binary utf8 encoded string of characters limited to 236 bytes, this can be treated as the body of the message and may be used to handle client-side concerns, such as intended recipient or requested map tile.    

An example messge may appear as follows,
`0100c|<noffle>@juul did you feel that earthquake!`

Alternatively, you could write another Layer3 client for as part of the disater radio firmware and create your own Layer 4 message format. See more about our networkng stack on our wiki, https://github.com/sudomesh/disaster-radio/wiki/Layered-Model.

# Uploading to dev board 

Follow instruction in main [README.md](https://github.com/sudomesh/disaster-radio#building-web-app) to upload built web app.

