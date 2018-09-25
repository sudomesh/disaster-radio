
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
    
    join(e) {
      e.preventDefault()

      var inp = document.getElementById('nickInput');

      app.actions.chat.join(inp.value, function(err) {
        if(err) {
          console.error(err);
          return;
        }

        document.getElementById('joinForm').style.display = 'none';
        document.getElementById('chatForm').style.display = 'block';
        document.getElementById('chatInput').focus();
      });
    }

    send(e) {
      e.preventDefault()
      var inp = document.getElementById('chatInput')

      app.actions.chat.sendMessage(inp.value, function(err) {
        if(err) {
          console.error(err);
          return;
        }

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
        <form id="joinForm" onsubmit={this.join.bind(this)}>
          <input id="nickInput" type="text" placeholder="Enter your name to join" autofocus />
          <input id="joinSubmit" type="submit" value="Join" />
        </form>

        <form id="chatForm" action="/chat" method="POST" onsubmit={this.send.bind(this)}>
          <div id="chat">
            {messages}
          </div>
          <input id="chatInput" type="text" name="msg"/>
        </form>
      </div>
	  }
  }
}
