
var arrayBufferToBuffer = require('arraybuffer-to-buffer');

module.exports = function(uriPath, opts) {
  opts = opts || {};
  opts.msgIDLength = 8
  this.opts = opts;
  

  var wsProtocol = 'ws://';
  if(window.location.protocol.match(/^https/i)) {
    wsProtocol = 'wss://';
  }

  this.url = wsProtocol + window.document.location.host + uriPath;

  this.connected = false;

  this._listeners = [];
  this._sentCallbacks = {};

  this._id = function(len) {
    len = len || this.opts.msgIDLength;
    var b = Buffer.alloc(len);
    var i;
    for(i=0; i < len; i++) {
      b[i] = Math.floor(Math.random() * 256);
    }
    return b;
  }

  this._toHex = function(b) {
    var s = ''
    var c;
    var i;
    for(i=0; i < b.length; i++) {
      s += (b[i] <= 16) ? '0'+b[i].toString(16) : b[i].toString(16);
    }
    return s;
  }

  this._stateChange = function(err) {
    var isConnected = (this.socket.readyState === 1) ? true : false;
    if(isConnected !== this.connected) {
      this.connected = isConnected;
      if(isConnected) err = undefined;
      this.connectCb(err, isConnected);
      if(!isConnected) {
        this.reconnect();
      } else {
        this.attempt = 0;
      }
    }
  };

  // route incoming messages to the correct listeners
  this._gotMessage = function(event) {
    if(!event.data) {
      console.warn("Received message with no data");
      return;
    }
    var data = arrayBufferToBuffer(event.data);

    if(data.length <= this.opts.msgIDLength) {
      console.warn("Received invalid message (too short)");
      return;
    }

    var id = data.slice(0, this.opts.msgIDLength);
    data = data.slice(this.opts.msgIDLength);

    if(opts.debug) console.log("[websocket rx]", this._toHex(id), data.toString('utf8'))

    // is this an ACK?
    if((data.slice(0, 1).toString('utf8') === '!') && (data.length === 1)) {
      var sentCb = this._sentCallbacks[id];
      if(!sentCb) {
        console.warn("Got ACK for unknown message ID");
        return;
      }
      
      clearTimeout(sentCb.timeout);
      sentCb.callback();

      delete this._sentCallbacks[id];
      return;
    } 

    var i, l, namespace, split;
    for(i=0; i < this._listeners.length; i++) {
      l = this._listeners[i]
      if(data.indexOf(l.namespace) !== 0) continue;
      if(!(split = data.indexOf('|'))) {
        console.error("invalid message received: no namespace")
        continue;
      }
      if(data.length < split + 2) {
        console.error("invalid message received: empty message")
        continue;
      }

      namespace = data.slice(0, split);
      data = data.slice(split + 1);
      l.callback(namespace, data);
    }
  };

  this.connect = function(connectCb) {
    this.connectCb = this.connectCb || connectCb;
    this.socket = new WebSocket(this.url);
    this.socket.binaryType = 'arraybuffer';
    this.socket.onopen = this._stateChange.bind(this);
    this.socket.onerror = this._stateChange.bind(this);
    this.socket.onclose = this._stateChange.bind(this);
    this.socket.onmessage = this._gotMessage.bind(this);
  };

  this.reconnect = function() {
    this.attempt++;
    // exponential backoff
    var delay = Math.floor(Math.pow(2, this.attempt) * 1000);
    setTimeout(this.connect.bind(this), delay);
  };

  this.send = function(namespace, msg, cb) {
    var msgID = this._id()

    msg = Buffer.concat([msgID, Buffer.from(namespace + '|' + msg, 'utf8')])
    this.socket.send(msg)

    this._sentCallbacks[msgID] = {
      callback: cb,
      timeout: setTimeout(function() {
        if(!this._sentCallbacks[msgID]) return
        
        cb(new Error("Timed out while waiting for ack"));
      }.bind(this), 5000)
    };
  };

  this.addListener = function(namespace, cb) {
    if(!Buffer.isBuffer(namespace)) {
      namespace = Buffer.from(namespace, 'utf8');
    }
    
    this._listeners.push({
      namespace: namespace,
      callback: cb
    });
  };

}

