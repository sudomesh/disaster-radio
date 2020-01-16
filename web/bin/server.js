#!/usr/bin/env node

var fs = require('fs');
var path = require('path');
var net = require('net');
var http = require('http');
var minimist = require('minimist');
var ecstatic = require('ecstatic')
var WebSocket = require('ws');
var router = require('routes')(); // server side router

var argv = minimist(process.argv.slice(2), {
  alias: {
    d: 'debug',
    p: 'port',
    s: 'settings'
  },
  boolean: [
    'debug'
  ],
  default: {
    settings: '../settings.js',
    home: path.dirname(__dirname),
    port: 8000
  }
});

var settings = require(argv.settings);
settings.home = argv.home || settings.home;
settings.port = argv.port || settings.port;

var staticFiles = ecstatic({
  root: path.join(settings.home, 'static'),
  baseDir: '',
  gzip: true,
  cache: 0
});

router.addRoute('/*', function(req, res, match) {
  return staticFiles(req, res);
});


var server = http.createServer(function (req, res) {
  var m = router.match(req.url);
  m.fn(req, res, m);
});

var wsServer = new WebSocket.Server({
  server: server,
  path: '/ws'
});

function genID(curID) {
  var b = Buffer.alloc(2);
  b.writeUInt16LE(curID);

  if(curID >= (Math.pow(2, 16) - 1)) {
    curID = 0;
  } else {
    curID++;
  }

  return {
    id: b,
    nextID: curID
  }
}


function send(ws, msg, curID) {
  var o;
  if(!Buffer.isBuffer(curID)) {
    o = genID(curID);
  }

  var msg = Buffer.concat([((o) ? o.id : curID), msg]);
  
  ws.send(msg, {
    compress: false,
    binary: true
  });

  if(o) {
    return o.nextID;
  }
}

function sendMsg(ws, msg, curID) {

  if(!Buffer.isBuffer(msg)) {
    msg = Buffer.from(msg, 'utf8');
  }

  msg = Buffer.concat([Buffer.from('c|', 'utf8'), msg]);

  return send(ws, msg, curID);
}

function sendRte(ws, msg, curID) {

  if(!Buffer.isBuffer(msg)) {
    msg = Buffer.from(msg, 'utf8');
  }

  msg = Buffer.concat([Buffer.from('r|', 'utf8'), msg]);

  return send(ws, msg, curID);
}

function sendACK(ws, msg) {
  var msgID = msg.slice(0, 2);

  return send(ws, Buffer.from('!', 'utf8'), msgID);
}


wsServer.on('connection', function(ws, req) {

  var curID = 0;

  // send fake chat messages every so often
  var sendTimer = setInterval(function() {
    console.log("sending message with ID:", curID);
    curID = sendMsg(ws, "<cookie_cat> hello apocalypse!", curID);
    console.log("sending routes with ID:", curID);
    var testRoutes = [0xd8, 0xa0, 0x1d, 0x69, 0xbd, 0x4c, 0x01, 0xFF, 0xd8, 0xa0, 0x1d, 0x69, 0xfc, 0x48, 0x01, 0xFF];
    curID = sendRte(ws, testRoutes, curID);

  }, 5000);

  ws.on('close', function(code, reason) {
    console.log("client disconnected");
    clearInterval(sendTimer);
  });
 
  ws.on('message', function(message) {

    console.log('received: %s', message.toString('utf8'));

    // send ACK
    setTimeout(function() {

      sendACK(ws, message);

    }, 500)
  });
 
});

// start the webserver
console.log("Starting disaster radio simulator on " + (settings.hostname || '*') + " port " + settings.port);
server.listen(settings.port, settings.hostname);
