
This is the web app served up by the ESP8266 for Disaster Radio + the simulator web server.

# Setup

```
npm install
cp settings.js.example settings.js
```

# Building and running

```
npm run build # build the js and css
npm start # run the simulator server
```

Then in a browser open http://localhost:8000/

# Developing

```
npm run dev # starts simulator server and auto-builder with HMR
```

or if you don't like HMR:

```
./bin/dev.js --cold
```

# Uploading to ESP8266

ToDo



