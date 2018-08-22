
This is the web app served up by the ESP8266 for Disaster Radio + the simulator web server.

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

# Uploading to ESP8266

After building with npm, the web app can be uploaded with `make flash_fs` from repository root



