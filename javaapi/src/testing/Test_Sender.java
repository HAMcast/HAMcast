package testing;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;
import java.net.URI;
import java.util.ArrayList;
import java.util.List;


import libHamCast.HamcastUtility;
import libHamCast.Log;
import libHamCast.MulticastSocket;
import multicastApi.NetworkMInterface;

public class Test_Sender {

	/**
	 * @param args
	 * @throws IOException
	 */
	public static void main(String[] args){

	

		try {
		URI uri = new URI("ip://239.0.0.1:1234");
		//List<NetworkMInterface> list=HamcastUtility.getAllInterfaces();
		
		MulticastSocket mSocket = new MulticastSocket();
		/*System.out.println("mSocket interfaces:"+mSocket.getInterfaces());
		int localhostid=23;
		List<NetworkMInterface> bla=HamcastUtility.getAllInterfaces();
		for(NetworkMInterface nmi: bla){
			if(nmi.getDisplayName().startsWith("lo"))localhostid=nmi.getIndex();
		}
		
		List<Integer> ids=new ArrayList<Integer>();
		ids.add(localhostid);
		mSocket.setInterfaces(ids);*/
		
		//List<NetworkMInterface> ifaces=mSocket.getInterfaces();

		//System.out.println(mSocket.getInterfaces());
		

	/*	for (int i = 0; i < 100; i++) {

			String hello_world = "Hello World " + "Nr. " + i;
			
			mSocket.send(uri, hello_world.getBytes());
			System.out.println(hello_world);
			
			mSocket.flush(uri.toString());
			Thread.sleep(10);} */
	//	mSocket.send(uri,"blub".getBytes());
		File file = new File("/home/berg/Bilder/lorem");

		//byte[] b = new byte[(int) file.length()];
		

			FileInputStream fileInputStream = new FileInputStream(file);
			byte[] b = new byte[(int)file.length()];
			int read= fileInputStream.read(b);
			System.out.println("read: "+read);
			System.out.println(mSocket.send(uri, b));

			
	
			 
			 System.out.println("starting sending");
		/*	for(long i=0;i<file.length();i=i+2){
				byte[] b = new byte[2];
				int read=0;
				if(file.length()==i+1){
					read=fileInputStream.read(b,0,1);
					mSocket.send(uri,b);
					break;
				}
				read=fileInputStream.read(b,0,2);
				mSocket.send(uri,b);
				
				 System.out.println("read: "+read);        
			}*/
			 
			 
			
		 //mSocket.send(uri,"helloworld".getBytes());
			 System.out.println("ready sending");
		} catch (Exception e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		} 
		
	
	
	
}
}
		

		

	


