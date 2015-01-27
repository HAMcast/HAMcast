package examples;

import libHamCast.MulticastSocket;

public class SimpleReceiver {

	public static void main(String[] args) throws Exception{
		//create an multicast socket object
		MulticastSocket mSocket=new MulticastSocket();


		
		mSocket.join("ip://239.0.0.1:1234");
		System.out.println("joined ip://239.0.0.1:1234");

		while(true){
			String str=new String(mSocket.receive());
			System.out.println(str);
		}
	}
}
