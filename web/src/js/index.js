
import {h, render, Component, createElement} from 'preact'
import Chat from './chat.js'
import app from './app.js'
import Socket from './socket.js'


function renderAll() {
  var container = document.getElementById('container');
  var replace;

  if(container.children.length) {
    replace = container.children[0];
  }

  render(<Chat />, container, replace);
}


function init() {
  app.socket = new Socket('/ws', function(msg) {
    console.log("GOT:", msg);
  });

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



