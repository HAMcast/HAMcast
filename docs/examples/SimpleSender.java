package examples;

import java.net.URI;
import libHamCast.MulticastSocket;


public class SimpleSender {
	public static void main(String[] args){

		try{
			URI uri = new URI("ip://239.0.0.1:1234");
			MulticastSocket mSocket=new MulticastSocket();
			
			for (int i = 0; i < 100; i++) {

				String hello_world = "Hello World " + "Nr. " + i;

				mSocket.send(uri, hello_world.getBytes());
				System.out.println(hello_world);

				Thread.sleep(10);}
		}
		catch(Exception e){
			e.printStackTrace();
		}

	}
}
