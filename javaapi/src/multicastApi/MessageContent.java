package multicastApi;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.ObjectOutput;
import java.io.ObjectOutputStream;
import java.io.Serializable;
import java.net.URI;

/**Message Content describes the content of an message which is received from a multicast group. */
public class MessageContent implements Serializable {

	URI adress;
	byte[] content;
	
	public MessageContent(URI uri, byte[] content) { 
		this.adress=uri;
		this.content=content;
	}

	/**identifies the group. */
	public URI getAdress() {
		return adress;
	}

	/**this is what was send. */
	public byte[] getContent() {
		return content;
	}
	
	/** represents uri and message content as a byte array. you shouldn't need it.*/
	public byte[] asBytes() {
		byte[] yourBytes =null;
	
		try{
		ByteArrayOutputStream bos = new ByteArrayOutputStream();
		ObjectOutput out = new ObjectOutputStream(bos);   
		out.writeObject(this);
		yourBytes = bos.toByteArray(); 

		out.close();
		bos.close();
		}catch(Exception e){
		e.printStackTrace();
		}
		
		return yourBytes;
	}
}
