
import {h} from 'preact'

module.exports = function(Component) {

  var ChatMessage = require('./chat_message')(Component)
  var RouteMessage = require('./route_message')(Component)

  return class Chat extends Component {

    constructor(props) {
      super(props)

      this.setState({
        messages: [],
        routes: []
      });
      app.socket.addListener('c', this.receive)
      app.socket.addListener('r', this.receiveRoutes)
    }
    
    scrollBottom() {
      var chat = document.getElementById('chat');
      chat.scrollTop = chat.scrollHeight;
    }

    receive(namespace, data) {
      app.actions.chat.showMessage(data.toString('utf8'));
    }

    receiveRoutes(namespace, data) {
      var dataStr = '';
      // tried to map across data, but wasn't working, so just did a loop
      // convert byte array to byte string
      for(var i = 0 ; i < data.length; i++){
        var firstHalf = data[i] >> 4;
        var secondHalf = data[i] & parseInt(1111, 2);
        dataStr += firstHalf.toString(16) + secondHalf.toString(16);
      }
      app.actions.chat.showRoutes(dataStr.toString('utf8'));
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

      var routes = this.state.routes.map(function(o) {
        return <RouteMessage rts={o} />
      }, this)

		  return <div>
        <form id="chatForm" action="/chat" method="POST" onsubmit={this.send}>
          <div id="chat">
            {messages}
          </div>
          <div id="routes">
            <div><u>Active Nodes | Hops | Metric </u></div>
            {routes}
          </div>
          <input id="chatInput" type="text" name="msg" placeholder="Enter your name or alias" autofocus />
        </form>
      </div>
	  }
  }
}
