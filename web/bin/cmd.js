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

wsServer.on('connection', function(ws, req) {
 
  wsServer.on('message', function(message) {
    console.log('received: %s', message);
  });
 
});

// start the webserver
console.log("Starting disaster radio simulator on " + (settings.hostname || '*') + " port " + settings.port);
server.listen(settings.port, settings.hostname);
