package testing;

import java.net.URI;
import java.util.List;

import libHamCast.HamcastUtility;
import libHamCast.MulticastSocket;
import multicastApi.NetworkMInterface;

public class TestSendingMoreData {

	public static void main(String[] args){

		try{


		URI uri = new URI("ip://239.0.0.1:1234");
		List<NetworkMInterface> list=HamcastUtility.getAllInterfaces();
		
		MulticastSocket mSocket = new MulticastSocket();
		System.out.println("mSocket interfaces:"+mSocket.getInterfaces());
		/*int localhostid=23;
		List<NetworkMInterface> bla=HamcastUtility.getAllInterfaces();
		for(NetworkMInterface nmi: bla){
			if(nmi.getDisplayName().startsWith("lo"))localhostid=nmi.getIndex();
		}
		
		List<Integer> ids=new ArrayList<Integer>();
		ids.add(localhostid);
		mSocket.setInterfaces(ids);*/
		System.out.println(mSocket.getInterfaces());

		for (int i = 0; i < 100; i++) {

			String hello_world = "Hello World " + "Nr. " + i;
			
			mSocket.send(uri, hello_world.getBytes());
			System.out.println(hello_world);
			
			mSocket.flush(uri.toString());
			Thread.sleep(10);}
		}
		catch(Exception e){
			e.printStackTrace();
		}
		
		

		}
}
