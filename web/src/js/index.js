
import {h, render, Component as PreactComponent} from 'preact'
import Socket from './socket.js'
import ashnazg from 'ashnazg'
import cipher from './cipher.js'

const Component = ashnazg(PreactComponent)
var Chat = require('./components/chat.js')(Component)

function renderAll() {
  var container = document.getElementById('container');
  var replace;

  if(container.children.length) {
    replace = container.children[0];
  }

  render(<Chat state />, container, replace);
}




function init() {
  app.socket = new Socket('/ws', {debug: true});

  app.socket.connect(function(err, isConnected) {
    if(err) console.error(err);    

    console.log("connected:", isConnected);

  });
  
  app.actions = require('./actions/index');

  renderAll();
  try {
    cipher.init();
  } catch(e) {
    console.error(e);
    alert("Fatal error:", e);
  }
}


init();



