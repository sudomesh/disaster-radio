
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
    
    scrollBottom() {
      var chat = document.getElementById('chat');
      chat.scrollTop = chat.scrollHeight;
    }

    receive(namespace, data) {
      app.actions.chat.showMessage(data.toString('utf8'));
    }

    send(e) {
      e.preventDefault()
      var inp = document.getElementById('chatInput')

      app.actions.chat.sendMessage(inp.value, function(err) {
        if(err) return;

        inp.value = '';
        inp.placeholder = '';
      });
    }

    componentDidUpdate() {
      this.scrollBottom()
    }

	  render() {

      var messages = this.state.messages.map(function(o) {
        // TODO is there a simpler way of passing a bunch of properties?
        return <ChatMessage txt={o.txt} type={o.type} />
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
