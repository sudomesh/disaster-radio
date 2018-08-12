
import cipher from '../cipher.js';

var self = module.exports = {

  showMessage: function(txt, type) {
    if(!type) type = 'remote';
    var msgs;
    if(!app.state.chat.messages) {
      msgs = []
    } else {
      msgs = app.state.chat.messages.slice(0);
    }

    msgs.push({txt: txt, type: type});
    app.changeState({chat: {messages: msgs}});
  },

  join: function(nick) {
    app.changeState({
      user: { 
        name: nick
      }
    })
  },

  sendMessage: function(msg, cb) {
    if(!msg.trim()) return cb(new Error("You must supply a (non-whitespace) message/nick"));

    var type = 'self';

    if(!app.state.user || !app.state.user.name) {
      self.join(msg);
      msg = '~ ' + msg + ' joined the channel';
      type = 'status';
    } else {
      msg = '<'+app.state.user.name+'> ' + msg;
    }

    // TODO also send the signature
    var signature = cipher.sign(msg);
    console.log("Signature:", signature);

    // <16 bytes random id><single byte msg type / namespace>|<actual message>
    app.socket.send('c', msg, function(err) {
      if(err) {
        console.error("Failed to send:", err) // TODO handle better
        return cb(err)
      }

      self.showMessage(msg, type);

      cb();
    });
  }
};
