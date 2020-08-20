
import {h} from 'preact'

module.exports = function(Component) {

  return class RouteMessage extends Component {

    constructor(props) {
      super(props)

      this.state = {}
    }
    
	  render() {
		  return <div class={this.props.type}>{this.props.rts.mac} |  {this.props.rts.hops}  |   {this.props.rts.metric}</div>

	  }
  }
}
