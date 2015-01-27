package testing;


import java.io.BufferedWriter;
import java.io.File;
import java.io.FileOutputStream;
import java.io.FileWriter;
import java.io.OutputStream;
import java.net.URI;
import java.util.ArrayList;
import java.util.List;
import libHamCast.HamcastUtility;
import libHamCast.Ipc;
import libHamCast.Log;
import libHamCast.MulticastSocket;
import multicastApi.MessageContent;
import multicastApi.NetworkMInterface;

public class Test_Receiver {

	public static void main(String[] args) throws Exception {
		
		Log.logLevel=Log.TRACE;

		URI uri;

		uri = new URI("ip://239.0.0.1:1234");
		NetworkMInterface nmi1= new NetworkMInterface(2, "lo", uri.toString(),"loopback");
		MulticastSocket mSocket = null;
		mSocket = new MulticastSocket();
		MulticastSocket	mSocket2 = new MulticastSocket();
		MulticastSocket	mSocket3 = new MulticastSocket();
		

		List<NetworkMInterface> list = HamcastUtility.getAllInterfaces();

	/*	for(NetworkMInterface nmi:list) {

			

			System.out.println("index: " + nmi.getIndex());
			System.out.println("load: " + nmi.getDisplayName());
			System.out.println("adress: " + nmi.getInetAddress());
			System.out.println("technology: " + nmi.getTech() + "\n");

		}*/

		List<NetworkMInterface> interfacesFromOneSocket=mSocket.getInterfaces();
		for(NetworkMInterface nmi: interfacesFromOneSocket){
			System.out.println("intialising: interfaceNr: "+nmi);
		}
	
		
		NetworkMInterface nmi2=HamcastUtility.getAllInterfaces().get(1);
		System.out.println("nmi2: "+nmi2);
	
		boolean klappt=mSocket.addInterface(nmi1.getIndex());
		System.out.println("added Interface: "+klappt);
			
			interfacesFromOneSocket=mSocket.getInterfaces();
			for(NetworkMInterface nmi: interfacesFromOneSocket){
				System.out.println("after  adding: interface: "+nmi);
			}
			
			boolean klappt1=mSocket.delInterface(nmi2.getIndex());
	System.out.println("deleted interface: "+klappt1);
	
	interfacesFromOneSocket=mSocket.getInterfaces();
	for(NetworkMInterface nmi: interfacesFromOneSocket){
		System.out.println("after Delete: "+nmi);
		
	}

	List<Integer> newInterfaces=new ArrayList<Integer>();
	//newInterfaces.add(nmi2);
	newInterfaces.add(nmi1.getIndex());
	boolean klappt2=mSocket2.setInterfaces(newInterfaces);
	System.out.println("set Interfaces: "+klappt2);
	List<NetworkMInterface> interfaces=mSocket2.getInterfaces();
	for(NetworkMInterface nmi: interfaces){
		System.out.println("after Setting: "+nmi);
		
	}
	
	

		mSocket.join(uri);
		
		mSocket.getInterfaces();
		
		list = HamcastUtility.getAllInterfaces();

		//Try and set TTL
		boolean settedTTL=Ipc.getIpc().setTTL(223,1);
		System.out.println("setted ttl: "+settedTTL);
		
		
		System.out.println("socket");
		for(NetworkMInterface nmi:list) {

			

			System.out.println("index: " + nmi.getIndex());
			System.out.println("name: " + nmi.getDisplayName());
			System.out.println("adress: " + nmi.getInetAddress());
			System.out.println("technology: " + nmi.getTech() + "\n");

		}
		System.out.println(HamcastUtility.getExceptionHandler());
		
//		File file=new File("/home/berg/newMusic.tar.gz");
		File file=new File("/home/berg/Bilder/lorem2");
		OutputStream out = new FileOutputStream(file);

	//	while (file.length()<53067) {
			
			
			System.out.println("start receiving");
			MessageContent mess=mSocket.receive();
			System.out.println("received");
			out.write(mess.getContent());
		
		//	String str=new String(mess.getContent());
		//	System.out.println(str);

//	}
			out.close();
		
		}

	
}	


