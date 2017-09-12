
module.exports = {

  showMessage: function(txt) {
    var msgs = app.state.chat.messages.slice(0);
    msgs.push(txt);
    app.changeState({chat: {messages: msgs}});
  },

  sendMessage: function(msg, cb) {
    
    if(msg) {
      if(!app.state.user || !app.state.user.name) {
        app.changeState({
          user: { 
            name: msg
          }
        })
        return cb();
      }
    }

    app.socket.send('c', msg, function(err) {
      if(err) {
        console.error("Failed to send:", err) // TODO handle better
        return cb(err)
      }

      app.actions.chat.showMessage('<'+app.state.user.name+'> ' + msg);

      cb();
    });
  }
};
