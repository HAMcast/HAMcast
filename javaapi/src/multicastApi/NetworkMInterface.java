package multicastApi;

/**
 * One Instance of this class represents one MulticastInterface
 * It is used to identify the local interface on which the Multicastgroup is joined.
 * It's the returnvalue in for some methods.
 * **/
public class NetworkMInterface {

	private int index;
	private String name;
	private String address;
	private String tech;
	
	public NetworkMInterface(int index, String name, String address, String tech) {
		this.index = index;
		this.name = name;
		this.address = address;
		this.tech = tech;

	}



	/**@return address of the local interface*/
	public String getInetAddress() {

		return address;
	}

	/**@return name of the local interface */
	public String getDisplayName() {

		return name;
	}

	/**identifier of the local interface */
	public int getIndex() {

		return index;
	}
	
	/**@return the technology which the interface uses. */
	public String getTech() {

		return tech;
	}
	
	@Override
	public String toString(){
		return "[index:"+this.index+", name:"+this.name+", adress:"+this.address+", tech:"+this.tech+"]";
	}
	
	@Override
	public boolean equals(Object obj){
		if (obj instanceof NetworkMInterface){
            NetworkMInterface nmi=(NetworkMInterface)obj;
            if(	nmi.index==this.index 
            	/*	&&
            	nmi.address.equals(this.address) &&
            	nmi.tech.equals(this.tech) &&
            	nmi.name.equals(this.name)*/
            	)
            {return true;}
           }
       return false;
	}

}
