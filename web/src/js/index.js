
import {h, render, Component as PreactComponent} from 'preact'
import Socket from './socket.js'
import ashnazg from 'ashnazg'

const Component = ashnazg(PreactComponent)
var Chat = require('./chat.js')(Component)

function renderAll() {
  var container = document.getElementById('container');
  var replace;

  if(container.children.length) {
    replace = container.children[0];
  }

  render(<Chat />, container, replace);
}


function init() {
  app.socket = new Socket('/ws', {debug: true});

  app.socket.connect(function(err, isConnected) {

    console.log("connected:", isConnected);
    if(err) console.error(err);    

  });
  
  renderAll();
}



// hot module reloading
if(module.hot) {
  module.hot.accept();
}

init();



