
import {h, render, Component as PreactComponent} from 'preact'
import Socket from './socket.js'
import ashnazg from 'ashnazg'

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

    console.log("connected:", isConnected);
    if(err) console.error(err);    

  });
  
  app.actions = require('./actions/index');

  renderAll();
}



// hot module reloading
if(module.hot) {
  module.hot.accept();
}

init();



