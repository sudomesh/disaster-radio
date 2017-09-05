
module.exports = function(uriPath, msgCb) {

  var wsProtocol = 'ws://';
  if(window.location.protocol.match(/^https/i)) {
    wsProtocol = 'wss://';
  }

  this.url = wsProtocol + window.document.location.host + uriPath;

  this.connected = false;

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

  this.connect = function(connectCb) {
    this.connectCb = this.connectCb || connectCb;
    this.socket = new WebSocket(this.url);
    this.socket.binaryType = 'arraybuffer';
    this.socket.onopen = this._stateChange.bind(this);
    this.socket.onerror = this._stateChange.bind(this);
    this.socket.onclose = this._stateChange.bind(this);
    this.socket.onmessage = msgCb;
  };

  this.reconnect = function() {
    this.attempt++;
    var delay = Math.floor(Math.pow(2, this.attempt) * 1000);

    setTimeout(this.connect.bind(this), delay);
  }
}

