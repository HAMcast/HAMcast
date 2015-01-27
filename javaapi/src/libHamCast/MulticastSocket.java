package libHamCast;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.URI;
import java.net.URISyntaxException;

import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ConcurrentMap;
import java.util.concurrent.LinkedBlockingQueue;
import java.util.concurrent.locks.Condition;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;
import java.util.concurrent.locks.ReentrantReadWriteLock;

import multicastApi.MessageContent;
import multicastApi.NetworkMInterface;

/**
 * implementing class of the MulticastSocket interface, 
 * For binding the application to the HAMcast middleware. <br>
 * 
 * \section exception Exception-Handling with Runnable
 * At {@link setCheckConnectionRunnable(Runnable r)} you can set the piece of code which will be executed, if an unexpected error occurs.
 * The default exception handler writes an IOException in the java EventQueue. For example, this exception will be appear if the middleware is shutting down while the java api uses it.
 * 
 * <b>Some functions throw IOException</b> - more information with [ExceptionName].getMessage() and at {@link libHamCast.Proxy#checkErr(long, String, String)}
 * @see libHamCast.Proxy#checkErr(long, String, String)
 * 
 *
 * 
 */

public class MulticastSocket {

	private Ipc ipc;
	

	//global list containing all MulticastSocket elements.
	protected static ConcurrentMap<Integer, MulticastSocket> socketList = new ConcurrentHashMap<Integer, MulticastSocket>();

	private int socketID;

	public final ReentrantLock sendlock = new ReentrantLock();
	public Condition sendCondition = sendlock.newCondition();

	public final ReentrantLock receivelock = new ReentrantLock();
	public Condition receiveCondition = receivelock.newCondition();

	// Blockierende Queue, in diese werden die Empfangenden Packete gelegt.
	//blocking queue, in which the receiving packages are putted in. 
	public LinkedBlockingQueue<MessageContent> receiveBlockQueue;

	// Größe der Queue, damit diese nicht unendlich wächst
	// current size of the Bufferqueue, for not growing into infinity
	public int receiveBufferSize;

	// mutex damit auf receiveBufferSize Threadsicher hoch und runter gezählt
	// werden kann
	/*
	 * mutex for counting receiveBufferSize threadsafe
	 * */
	protected final ReentrantReadWriteLock receiveLock = new ReentrantReadWriteLock();
	protected final Lock readLock = receiveLock.readLock();
	protected final Lock writeLock = receiveLock.writeLock();

	// Enthält alle Sendbuffer, die mit der Uri als Key referenziert werden
	// können
	/*
	 * contains all sendBuffers with uri as key
	 * */
	protected ConcurrentHashMap<String, Sendbuf> socksSendMap;
	
	
	// Enthält alle Sendbuffer, die mit dem Key stream_id referrenziert werden
	// können
	/*
	 * contains all sendbuffers with steam_id as key
	 * */
	protected ConcurrentHashMap<Integer, Sendbuf> socksSendMap_StreamID;

	// Beim laden wird einmalig die IPC Kommunikation aufgebaut
	/*
	 * uniquely loading the interprocesscommunication
	 * */
	

	/**
	 * @throws IOException if socket couldn't be created
	 */
	public MulticastSocket() throws IOException {
		ipc=Ipc.getIpc();
		socketID =(int)ipc.createSocket();
	
		socksSendMap = new ConcurrentHashMap<String, Sendbuf>();

		// für ein schnelles Mapping beim Empfangen eines Ack
		//for fast mapping when receiving an ACK
		socksSendMap_StreamID = new ConcurrentHashMap<Integer, Sendbuf>();

		receiveBlockQueue = new LinkedBlockingQueue<MessageContent>();
		MulticastSocket.socketList.put(socketID, this);
		// legt Receivebuffer Size für das Socket fest
		//first size of receiveBuffer
		receiveBufferSize = Parameter.ASYN_RCVBUF;

	}

	/**
	 * @param nmi  expects an instance NetworkMInterface 
	 * @throws IOException if socket could not be created
	 */
	public MulticastSocket(NetworkMInterface nmi) throws IOException {
		ipc=Ipc.getIpc();
		socksSendMap = new ConcurrentHashMap<String, Sendbuf>();

		// Erstellt eine HashMap für schnelle Referenzierungen von Sendbuffern
		socksSendMap_StreamID = new ConcurrentHashMap<Integer, Sendbuf>();
		
		// Erstellt und bindet ein Socket ein ein konkretes Interface
		socketID = (int)ipc.createSocket();
		
		receiveBlockQueue = new LinkedBlockingQueue<MessageContent>();
		// legt das Socket in die Globaleliste ab
		MulticastSocket.socketList.put(socketID, this);
		List<Integer> nmis=new ArrayList<Integer>();
		nmis.add(nmi.getIndex());
		ipc.setSockInterfaces(this.socketID, nmis);
		
	
	}

	/**
	 * @param iter expects an  array of NetworkMInterfaces
	 */
	public MulticastSocket(NetworkMInterface[] iter) throws IOException {
		ipc=Ipc.getIpc();
		socketID =(int)ipc.createSocket();
	
		socksSendMap = new ConcurrentHashMap<String, Sendbuf>();

		// für ein schnelles Mapping beim Empfangen eines Ack
		//for fast mapping when receiving an ACK
		socksSendMap_StreamID = new ConcurrentHashMap<Integer, Sendbuf>();

		receiveBlockQueue = new LinkedBlockingQueue<MessageContent>();
		MulticastSocket.socketList.put(socketID, this);
		// legt Receivebuffer Size für das Socket fest
		//first size of receiveBuffer
		receiveBufferSize = Parameter.ASYN_RCVBUF;
		
		for(int i=0; i<iter.length; i++){
			this.addInterface(iter[i].getIndex());
		}
	}

	
	/**
	 * @throws IOException
	 * @see multicastApi.MulticastSocket#getInterfaces()
	 */
	public List<Long> getSockInterfaceIDs() throws IOException {

		return ipc.getSockInterfaceIDs(socketID);

	}
	
	
	/**
	 * 
	 * @param nmi - networkMInterface you wish to add
	 * @return	true, if it was successful
	 * @throws IOException
	 */
	public boolean addInterface(int interfaceID) throws IOException{
		List<NetworkMInterface> list=HamcastUtility.getAllInterfaces();
		NetworkMInterface chosen=null;
		for(NetworkMInterface nmi: list){
			if(nmi.getIndex()==interfaceID)chosen=nmi;
		}
		if(chosen==null)return false;
		boolean success=ipc.addSockInterface(this.socketID, chosen);
		return success;
	}
	
	/**
	 * overwrites the current interfaces which are used by the socket.
	 * @param nmi - List of Interfaces you wish to set, precondition:  nmi is not empty.
	 * @return true, if it was successful
	 * @throws IOExceptional
	 */
	public boolean setInterfaces(List<Integer> nmi) throws IOException{
		boolean success=ipc.setSockInterfaces(this.socketID, nmi);
		return success;
	}
	
	/**
	 * @return a List of the Interfaces which are set for this socket.
	 */
	public List<NetworkMInterface> getInterfaces() throws IOException {
		List<NetworkMInterface> sockInterfaces=new ArrayList<NetworkMInterface>();
		List<Long> sockInterfaceIDs=getSockInterfaceIDs();
		List<NetworkMInterface> interfaces= HamcastUtility.getAllInterfaces();
		for(NetworkMInterface nmi:interfaces){
			if(sockInterfaceIDs.contains(Long.valueOf(nmi.getIndex()))){
				sockInterfaces.add(nmi);
			}
			
		}
		
		
		return sockInterfaces;
	}
	
	/**
	 * removes an interface from the socket
	 * \warning use only InterfaceIDs which you got at {@link HamcastUtility#getAllInterfaces() getAllInterfaces}. Wrong Ids lead to UNDEFINDE BEHAVOUR
	  * @throws IOException if interfaces could not be deleted
	  * 
	 */
	public boolean delInterface(int InterfaceId) throws IOException {
		return ipc.delInterfaces(InterfaceId, this.socketID);

	}



	/**
	 * @see multicastApi.MulticastSocket#join(java.net.URI)
	 * join a multicast group
	 */
	public void join(URI uri) throws IOException {

		ipc.join(socketID, uri);

	}

	/** 
	 * @see multicastApi.MulticastSocket#join(java.lang.String)

	 */
	public void join(String uri) throws URISyntaxException, IOException {

		join(new URI(uri));

	}

	/**
	 * @see multicastApi.MulticastSocket#leave(java.lang.String)
	  * @throws IOException
	 */
	public void leave(String uri) throws IOException, URISyntaxException {

		leave(new URI(uri));
	}

	/**

	 * @see multicastApi.MulticastSocket#leave(java.net.URI)
	 */
	public void leave(URI uri) throws IOException {

		ipc.leave(socketID, uri);

	}


	

	/**
	 * is blocked until something is received
	 * @see multicastApi.MulticastSocket#receive()
	 */
	public MessageContent receive() throws IOException {

		MessageContent receive = null;

		try {
		//	blocking reading
			receive = receiveBlockQueue.take();
			
		} catch (InterruptedException e) {
			e.printStackTrace();
			
		}

		writeLock.lock();
		receiveBufferSize = receiveBufferSize + receive.getContent().length;
		writeLock.unlock();

		return receive;

	}

	/**
	 * @see multicastApi.MulticastSocket#nonBlockingSend(java.lang.String, byte[])
	 * @throws IOException 
	 * @see Proxy#checkErr(long, String, String)
	 */
	public boolean nonBlockingSend(String uri, byte[] data) throws IOException,
			URISyntaxException {

		return nonBlockingSend(new URI(uri.toString()), data);

	}

	/**
	 * For instant sending (doesn't block like send)
	 * @return true, if immediate sending was successful
	 * @see multicastApi.MulticastSocket#nonBlockingSend(java.net.URI, byte[])
	 * @see multicastApi.MulticastSocket#send(java.net.URI, byte[])

	 */
	public boolean nonBlockingSend(URI uri, byte[] data) throws IOException {

		String uri_s = uri.toString();

		if (!socksSendMap.containsKey(uri_s)) {

			int streamID = ipc.createStream(socketID, uri_s);

			Sendbuf sendbuf = new Sendbuf(uri_s, streamID, socketID, this);

			socksSendMap.put(uri_s, sendbuf);

			socksSendMap_StreamID.put(streamID, sendbuf);

		}

		Sendbuf sendbuf = socksSendMap.get(uri_s);

		return sendbuf.add(data);

	}

	/**
	 * @param uri_s
	 */
	public void flush(String uri_s) {

		Sendbuf sendbuf = socksSendMap.get(uri_s);

		sendbuf.flush();

	}

	/**
	 * @see multicastApi.MulticastSocket#send(java.lang.String, byte[])
	 * 
	 *
	 */
	public void send(String uri, byte[] data) throws IOException,
			URISyntaxException {

		send(new URI(uri.toString()), data);

	}

	/**
	 *  This method can block if there is not enough free space in the buffer.
	 *  Use nonBlockingSend for a sending attempt which returns an message of success<br>
	 *  @warning the size of sending bytearrays is limited by technical enviroments to 65k, don't send 
	 *  larger bytearrays.
	 *  
	  * @throws IOException if Message couldn't be send
	 * @throws InterruptedException 
	 * @see multicastApi.MulticastSocket#send(java.net.URI, byte[])
	 */
	public boolean send(URI uri, byte[] data) throws IOException {
		
		//TODO:change size of data!!!!!!!
		boolean notTooBig=true;
		if(data.length>65000)notTooBig=false;
		
		String uri_s = uri.toString();

		if (!socksSendMap.containsKey(uri_s)) {
			int streamID = ipc.createStream(socketID, uri_s);

			Sendbuf sendbuf = new Sendbuf(uri_s, streamID, socketID, this);

			socksSendMap.put(uri_s, sendbuf);

			socksSendMap_StreamID.put(streamID, sendbuf);

		}

		Sendbuf sendbuf = socksSendMap.get(uri_s);

		

		while (!sendbuf.add(data)) {
			//no sending possible, waiting for signal by ACK Thread
		try {
				sendlock.lock();
				sendCondition.await();
				sendlock.unlock();
			} catch (InterruptedException e) {
				e.printStackTrace();
			}
			
	

		}
		return notTooBig;
	}

	
	
	/**
	 * sets time-to-live if underlaying network supports it.
	 * @return true, if middleware understood your request.
	 */
	public boolean setTTL(int hops) throws IOException {
		return ipc.setTTL(hops, this.socketID);
	}



	/**
	 * removes socket from global list and from socket.
	 * This method is responsible for clean destruction of an MulticastSocket object.
	 * It informs the HAMcastMiddleware and remove the socket from all global lists.
	 */
	public void destroy() throws IOException {
		try{
		ipc.destroy(socketID);
		socketList.remove(socketID);
		} catch(IOException e){
			throw new IOException(e.getCause());
		}
		
		
		

	}

	/**
	 *for cleaning purposes
	 */
	@Override
	 public void finalize() {

		try {
			ipc.destroy(socketID);
		} catch (IOException e) {
			e.printStackTrace();
		}

		socketList.remove(socketID);
	}





	

}
