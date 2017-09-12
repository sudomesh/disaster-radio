
import {h} from 'preact'

module.exports = function(Component) {

  return class ChatMessage extends Component {

    constructor(props) {
      super(props)

      this.state = {}
    }
    
	  render() {
		  return <div>
        <span>{this.props.txt}</span>
      </div>

	  }
  }
}
