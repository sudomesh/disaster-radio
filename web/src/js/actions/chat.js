
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

  join: function(nick, cb) {
    app.changeState({
      user: { 
        name: nick
      }
    })
    this.sendMessage("~ " + nick + " joined the channel", cb, 'status');
  },

  sendMessage: function(msg, cb, type) {
    if(!msg.trim()) return cb(new Error("You must supply a (non-whitespace) message/nick"));


    type = type || 'self';

    if(type !== 'status') {
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
