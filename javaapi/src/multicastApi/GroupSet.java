package multicastApi;

import java.net.URI;
/** 
 * Groupset is returnvalue in some functions in {@link libHamCast.HamcastUtility#groupSet(NetworkMInterface)}.
 * Uri describes the uri of a network interface and Type the type of the network interface 
 * */
public class GroupSet {

	private URI uri;
	private int type;

	public GroupSet(URI uri, int type) {

		this.type = type;
		this.uri = uri;
	}
	
	/**@return The uri of the interface */
	public URI getUri() {
		return uri;
	}

	/**
	 *  an Interface can have 3 different types:
	 * @return <ul>
	 * 	<li>0 = listener state
	 *  <li>1 = sender state
	 *  <li>2 = listener and sender state
	 * </ul>  
	 * */
	public int getType() {
		return type;
	}
	
	@Override
	public String toString(){
		return "uri: "+this.uri+", type: "+this.type;
	}

}
