
import {h} from 'preact'

module.exports = function(Component) {

  return class Chat extends Component {

    constructor(props) {
      super(props)

      this.state = {
        
      }
      app.socket.addListener('c', this.receive)
    }
    
    receive(namespace, data) {
      console.log("Received data:", data.toString())
    }

    send(e) {
      e.preventDefault()

      var input = document.getElementById('chatInput')
      var msg = input.value
      if(msg) {
        if(!app.state.user || !app.state.user.name) {
          app.changeState({
            user: { 
              name: msg
            }
          })
        }
      }
      app.socket.send('c', input.value, function(err) {
        if(err) return console.error("Failed to send:", err) // TODO handle better
        input.value = '';
      });
    }

	  render() {
		  return <div>
        <form id="chatForm" action="/chat" method="POST" onsubmit={this.send}>
        <div id="chat">
        </div>
        <input id="chatInput" type="text" name="msg" placeholder="Enter you name or alias" autofocus />
        </form>
        </div>
	  }
  }
}
