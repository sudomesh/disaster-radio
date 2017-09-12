
import {h} from 'preact'

module.exports = function(Component) {

  var ChatMessage = require('./chat_message')(Component)

  return class Chat extends Component {

    constructor(props) {
      super(props)

      this.setState({
        messages: []
      });
      app.socket.addListener('c', this.receive)
    }
    
    receive(namespace, data) {
      console.log("Received data:", data.toString())
    }

    send(e) {
      e.preventDefault()
      var inp = document.getElementById('chatInput')

      app.actions.chat.sendMessage(inp.value, function(err) {
        if(err) return;

        inp.value = '';
      });
    }

	  render() {

      var messages = this.state.messages.map(function(txt) {
          return <ChatMessage txt={txt} />
      }, this)

		  return <div>
        <form id="chatForm" action="/chat" method="POST" onsubmit={this.send}>
          <div id="chat">
            {messages}
          </div>
          <input id="chatInput" type="text" name="msg" placeholder="Enter you name or alias" autofocus />
        </form>
      </div>
	  }
  }
}
