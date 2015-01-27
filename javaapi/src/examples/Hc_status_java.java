package examples;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.net.URI;
import java.net.URISyntaxException;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.LinkedList;
import java.util.List;

import libHamCast.HamcastUtility;
import libHamCast.Log;
import libHamCast.MulticastSocket;
import multicastApi.GroupSet;
import multicastApi.NetworkMInterface;

public class Hc_status_java {

	//test some utility and socket functions.
	static String help="\n \nTools for testing Java HAMcast Multicast API\n\n" +
			"Usage:java -jar hc_status_java.jar [hamcast-uri1] [hamcast-uri2] [...]\n" +
			"Note: needs running HAMcast middleware with active loopback module \n\n" +
			"\t [hamcast-uriN] \t groups to join. if none, 1 default uri is used\n" +
			"\t -h, --help \t\t this help";
	
	public static void main(String[] args) {
		
		
		InputStreamReader istream = new InputStreamReader(System.in) ;
		BufferedReader bufRead = new BufferedReader(istream) ;
		List<String> uris=new LinkedList<String>();
		if(args.length==0){
			uris.add("ip://239.0.0.1:1234");
		}
		else if( "-h".equals(args[0]) || "--help".equals(args[0])){
			System.out.println(help);
			System.exit(0);
		} else {
			for( String str:args){
				uris.add(str);
			}
		}
		
		MulticastSocket socket=null;
		try {
			socket=new MulticastSocket();
		} catch (IOException e1) {
			System.out.println("Error: "+e1.getMessage());
		}

		//joining groups
		if(socket!=null){
			System.out.println("Joining groups...");

			if(!uris.isEmpty()){
				for( String str:uris){
					try {
						
						socket.join(str);
						System.out.println("join: "+str);
					} catch (URISyntaxException e) {
						System.out.println("uri-exception-uri wasn't right");
					} catch (IOException e) {
						System.out.println("Error Cause: "+ e.getCause());
						System.out.println("Error Message: "+ e.getMessage());
					}
			}
			}
		} else System.out.println("Socket was null, so joining is no option...");
		
		System.out.println("\n");
		try{
		
		/*Test Interface-Actions: */
	
	
	
			List<NetworkMInterface> IfacesS1=socket.getInterfaces();
			List<Long> IfaceIdS1=socket.getSockInterfaceIDs();
			System.out.println("socket-Interfaces:");
	
	
			//ADDING ONE INTERFACE
			for(NetworkMInterface nmi:IfacesS1){
				System.out.println("\t"+nmi);
				assert(IfaceIdS1.contains(Long.valueOf(nmi.getIndex())));
			}
			assert(IfaceIdS1.size()==IfacesS1.size());
	
	
			NetworkMInterface newNMI=new NetworkMInterface(2, "lo", "localhost","loopback");
	
			System.out.println("\nadding Interface: "+newNMI+
					"\n\tsuccess:"+socket.addInterface(newNMI.getIndex())+"\n");
	
	
			IfacesS1=socket.getInterfaces();
			IfaceIdS1=socket.getSockInterfaceIDs();
			System.out.println("socket-Interfaces:");
	
			for(NetworkMInterface nmi:IfacesS1){
				System.out.println("\t"+nmi);
				assert(IfaceIdS1.contains(Long.valueOf(nmi.getIndex())));
			} 	
			assert(IfaceIdS1.size()==IfacesS1.size());
	
	
	
			//DELETING ONE INTERFACE
			NetworkMInterface delIface=IfacesS1.get(1);
	
			System.out.println("\ndeleting Interface: "+delIface+
					"\n\tsuccess:"+socket.delInterface(newNMI.getIndex())+"\n");
	
			IfacesS1=socket.getInterfaces();
			IfaceIdS1=socket.getSockInterfaceIDs();
			System.out.println("socket-Interfaces:");
	
			for(NetworkMInterface nmi:IfacesS1){
				System.out.println("\t"+nmi);
				assert(IfaceIdS1.contains(Long.valueOf(nmi.getIndex())));
			}
			assert(IfaceIdS1.size()==IfacesS1.size());
	
	
			//SETTING TO ONLY LAST INTERFACE
			List<Integer> toSet=new ArrayList<Integer>();
			List<NetworkMInterface> allInterface=HamcastUtility.getAllInterfaces();
			System.out.println("Setting socket only to one Interface: "+ allInterface.get((allInterface.size()-1)).toString());
			toSet.add(allInterface.size()-1);
	
			System.out.println("\nsetting Interface: "+Arrays.toString(toSet.toArray())+
					"\n\tsuccess:"+socket.setInterfaces(toSet)+"\n");
	
			IfacesS1=socket.getInterfaces();
			IfaceIdS1=socket.getSockInterfaceIDs();
			System.out.println("socket-Interfaces:");
	
			for(NetworkMInterface nmi:IfacesS1){
				System.out.println("\t"+nmi);
				assert(IfaceIdS1.contains(Long.valueOf(nmi.getIndex())));
			}
			assert(IfaceIdS1.size()==IfacesS1.size());
	
	
			
			//if you want to receive this messages. join this groups before sending with another programm.
			
			  for(int j=0;j<23;j++){
				for(int i=1234; i<1244;i++){
					socket.send("ip://239.0.0.1:"+String.valueOf(i), ("from: "+i+"Hello World"+String.valueOf(j)).getBytes());
				}
			}
			
			
		//HamcastUtility-Functions
		System.out.println("\n\n------------HamcastUtility - Functions------------\n");
		System.out.println("designated Host: "+HamcastUtility.designatedHost(socket.getInterfaces().get(0), "ip://239.0.0.1:1234"));
		System.out.println("parents: "+HamcastUtility.parentSet(socket.getInterfaces().get(0), "ip://239.0.0.1:1234"));
		System.out.println("children: "+Arrays.toString(HamcastUtility.childrenSet(socket.getInterfaces().get(0), "ip://239.0.0.1:1234").toArray()));
	
		System.out.println("all Interfaces:");
		List<NetworkMInterface> allInterfaces=HamcastUtility.getAllInterfaces();
		for(NetworkMInterface nmi: allInterfaces){
			System.out.println("\t"+nmi);
		}
		
		System.out.println("\nactive registered multicast adresses (groupSet):");
		List<GroupSet> groups=HamcastUtility.groupSet(socket.getInterfaces().get(0));
		for(GroupSet g: groups){
			System.out.println("\t"+g);
		}
		
		System.out.println("\nneigborSet():");
		List<URI> neighbours = HamcastUtility.neighborSet(socket.getInterfaces().get(0));
		for(URI u: neighbours){
			System.out.println("\t"+u);
		}
		
			
			
			
			//changing the default exception handler
			HamcastUtility.setExceptionHandler(new Runnable() {
				
				@Override
				public void run() {
					System.out.println("your exception handling as you like it.");
					
				}
				@Override
				public String toString(){
					return "your handler";
				}
			});
			
			System.out.println("Exceptionhandler: "+HamcastUtility.getExceptionHandler());
			System.out.println("Leaving Groups");
			for( String str:uris){
			try {
				socket.leave(str);
				System.out.println("leave: "+str);
			} catch (URISyntaxException e) {
				System.err.println("couldn't leave uri: "+str);
				e.printStackTrace();
			}
			}
		} catch (Exception e){
			System.err.println("sorry, couldn't test utility functions...");
			e.printStackTrace();
		}
		
		
		System.out.println("Removing now socket");
		try {
			socket.destroy();
		} catch (IOException e) {
			System.err.println("sorry, couldn't destroy socket object");
			e.printStackTrace();
		
		
		}	

	}
	
	


}
