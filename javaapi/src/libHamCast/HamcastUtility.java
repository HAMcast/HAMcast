package libHamCast;

import java.io.IOException;
import java.net.URI;
import java.net.URISyntaxException;
import java.util.List;

import multicastApi.GroupSet;
import multicastApi.NetworkMInterface;


/**@mainpage Main 
 * \section intro_sec Introduction
 * This library provides an easy way for communication with the HAMcast middleware in Java. It provides two ways for communication.
 * On one hand, it is possible to create an multicast socket object which handles operations which needs additional information about groups to
 * join oder data to receive or to send. The class for this case is {@link libHamCast.MulticastSocket MulticastSocket}. On the other hand there are static utilitymethods in
 *  the class {@link libHamCast.HamcastUtility HamcastUtility} which offer more general information.
 *    
 *  
 * \section installation Installation
 * Download the Java library and add it to your build path.
 * 
 * \section starting Getting started
 * \subsection Workflow
 *
 * @image html images/senderAndReceiver2.png The workflow of an application using HAMcast java library. 
 * 
 * Following classes are a good start to look at, if you want to use HAMcast:
 * <ul> 
 * <li>{@link libHamCast.HamcastUtility} provides utility methods you can send to the middleware without using an explicit MulticastSocket.
 * <li>{@link libHamCast.MulticastSocket} Use this Socket to send and receive data.
 * </ul>
 * 
 * \subsection example Example 
 * You find simple examples {@link Examples here}
 
 * \subsection further Further Reading
 * have a look at http://hamcast.realmv6.org/developers/wiki/documentation
 * 
 * \subsection note Note
 * This library is a prototype.
 */

/**@page Examples Examples
 * \section introduction Examples
 * Here are two little example, to copy paste in your code to try the library.
 * Make sure your middleware is running (Have a look {@link http://hamcast.realmv6.org/developers here}).<br>
 * 
 * \subsection sender SimpleSender.java;
 * \code
 * public class SimpleSender {
	public static void main(String[] args){

		try{
			URI uri = new URI("ip://239.0.0.1:1234");
			MulticastSocket mSocket=new MulticastSocket();
			
			for (int i = 0; i < 100; i++) {
				
				String hello_world = "Hello World " + "Nr. " + i;
				
				//sending a message
				mSocket.send(uri, hello_world.getBytes());
				System.out.println(hello_world);

				Thread.sleep(10);}
		}
		catch(Exception e){
			e.printStackTrace();
		}

	}
}
 * \endcode
 * 
 * \subsection receiver SimpleReceiver.java
 * \code
 * public class SimpleReceiver {

	public static void main(String[] args) throws Exception{
		//create an multicast socket object
		MulticastSocket mSocket=new MulticastSocket();

		
		//join the group...
		mSocket.join("ip://239.0.0.1:1234");
		System.out.println("joined ip://239.0.0.1:1234");

		while(true){
			//...and receive the message
			String str=new String(mSocket.receive().getContent());
			System.out.println(str);
		}
	}
}
 * \endcode
 * 
 *  */


/** 
 * This class provides some tools for getting general information from the middleware, when no socket object is needed.
 * */
public class HamcastUtility {
	
	
private static Ipc ipc;
	
	// Static initializer
	static {
		try {
			
			exceptionHandler=new DefaultExceptionHandling();
			ipc = Ipc.getIpc();
		} catch (Exception e) {
			Log.log(Log.FATAL, "Middleware connection refused");
			Log.log(Log.FATAL, e.toString());
			
		}
	}
	
	private static Runnable exceptionHandler;
	
	/** 
	 * This Runnable will be invoked if an unexpected Exception occurs in the thread which receives data. For example, this will be invoked if the
	 * connection to the HAMcast middleware is closed.
	 * Default is {@link DefaultExceptionHandling DefaultExceptionHandling}.
	 * The thread will write this runnable to the event queue to invoke later.
	 * 
	 * @return the current exceptionHandler
	 * */
	public static Runnable getExceptionHandler(){
		return exceptionHandler;
	}
	
	/**
	 * setting your own exceptionHandler for the receiving thread. 
	 * @see getExceptionHandler
	 * @param eh
	 */
	public static void setExceptionHandler(Runnable eh){
		exceptionHandler=eh;
	}
	
	
	/**
	 * @param nmi
	 * @param uri
	 * @return List over all neighbors from which multicast data at a given interface (nmi) for the specified group (uri) are received.
	 * @throws IOException
	 * @throws URISyntaxException
	 */
	public static List<URI> parentSet(NetworkMInterface nmi, String uri)
			throws IOException, URISyntaxException {

		return parentSet(nmi, new URI(uri));

	}

	/**
	 * @param nmi
	 * @param uri
	 * @return
	 * @throws IOException
	 */
	public static List<URI> parentSet(NetworkMInterface nmi, URI uri)
			throws IOException {
		throwingNewIOException();
		return ipc.parentSet(nmi, uri);

	}

	/**
	 * @param nmi
	 * @param uri
	 * @return the set of child nodes that receive multicast Data from a specific interface (nmi) for a given group(uri)
	 * @throws IOException
	 * @throws URISyntaxException
	 */
	public static List<URI> childrenSet(NetworkMInterface nmi, String uri)
			throws IOException, URISyntaxException {

		return childrenSet(nmi, new URI(uri));

	}

	/** 
	 * @param nmi
	 * @param uri
	 * @return List all group Members who joined nmi
	 * @throws IOException
	 * @see {@link MulticastSocket#childrenSet(NetworkMInterface, String)}
	 */
	public static List<URI> childrenSet(NetworkMInterface nmi, URI uri)
			throws IOException {

		return ipc.childrenSet(nmi, uri);

	}

	/**
	 * @param nmi
	 * @return List<URI> over all known neighbors
	 * @throws IOException
	 */
	public static List<URI> neighborSet(NetworkMInterface nmi)
			throws IOException {

		return ipc.neighborSet(nmi);

	}

	/**
	 * 
	 * 
	 * @param nmi
	 * @return List over all active, registered multicast adresses
	 * @throws IOException
	 */
	public static List<GroupSet> groupSet(NetworkMInterface nmi)
			throws IOException {

		return ipc.groupSet(nmi);

	}

	/**
	 * @return List over all Interfaces which are available and active 
	 * @throws IOException
	 */
	public static List<NetworkMInterface> getAllInterfaces()
			throws IOException {
		throwingNewIOException();
		return ipc.getInterfaces();

	}

	/**
	 * @param nmi
	 * @param uri
	 * @return true, if this host is an forwarder or querier
	 * @throws IOException
	 * @throws URISyntaxException
	 */
	public static boolean designatedHost(NetworkMInterface nmi, String uri)
			throws IOException, URISyntaxException {

		return designatedHost(nmi, new URI(uri));
	}

	/**
	 * @param nmi
	 * @param uri
	 * @return true, if nmi is a forwarder resp. querier
	 * @throws IOException
	 * @see {@link MulticastSocket#designatedHost(NetworkMInterface, String)}
	 */
	public static boolean designatedHost(NetworkMInterface nmi, URI uri)
			throws IOException {

		return ipc.designatedHost(nmi, uri);

	}


	private static void throwingNewIOException() throws IOException{
		if(ipc==null)throw new IOException("could'nt initialize the ipc module, is your Middleware running? - check log for more details");
	}
	
	  private static int id=0;
	  static synchronized  long getNextNumber(){
			id++;
			return id;
		}
	
}
