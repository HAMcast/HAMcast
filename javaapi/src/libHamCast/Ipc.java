package libHamCast;


import java.io.IOException;
import java.net.URI;
import java.util.ArrayList;
import java.util.List;

import multicastApi.GroupSet;
import multicastApi.NetworkMInterface;


/** 
 * handles the IPC.
 * Static methods for seperation  of IPC internal things from socket.
 * This class is used by {@link MulticastSocket}. It's recommendable to use it for other MulticastSocket classes, too.
 * @see libHamCast.MulticastSocket
 * 
 * */
 public class  Ipc {

	Communication comModul;
	private static Ipc ref=null;

	private IpcReceiver ipcReceiver;
	// static damit set priority in IpcAsyncSendBuffer geht
	protected static IpcSender ipcSender;

	private Ipc(String adress) throws IOException  {
		
		comModul = new Communication(adress);

		
		// Sniffer Thread(Receiver) erstellen
		
		ipcReceiver = new IpcReceiver(comModul);
		ipcReceiver.setPriority(8);
		ipcReceiver.setDaemon(true);
		ipcReceiver.start();
		
		// Sender Thread starten
		ipcSender = new IpcSender(comModul);
		ipcSender.start();
		ref=this;
		


	}
	
	public static Ipc getIpc() throws IOException{
		if(ref==null){
			ref=new Ipc("localhost");
		}
		return ref;
	}
	
public Ipc(String adress, Runnable exceptionHandler) throws IOException  {

		comModul = new Communication(adress);

		
		// Sniffer Thread(Receiver) erstellen
		
		ipcReceiver = new IpcReceiver(comModul,exceptionHandler);
		
		ipcReceiver.setPriority(8);

		ipcReceiver.start();
		
		// Sender Thread starten
		ipcSender = new IpcSender(comModul);
		ipcSender.start();


	}
	

	
	
	/** 
	 * checks the error and throws I/O Exception in 2 cases <br>
	 * 
	 * 
	 * @throws I/O-Exception if <br>
	 * 1) Indicates that a requirement in the invoked function failed. <br>
	 * 2) Indicates that an unexpected exception in a middleware module occured. 
	 *
	 * */

	public void checkErr(long field1, String str, String str2)
			throws IOException {

		switch ((int) field1) {


			
		case (IDL.EID_REQUIREMENT_FAILED):

			String createdStringEID_REQUIREMENT_FAILED="Proxy::" + str + "::EID_REQUIREMENT_FAILED::" + str2;
			if (Log.ACTIVE) {
				Log.log(Log.ERROR, createdStringEID_REQUIREMENT_FAILED);
			}

			throw new IOException(createdStringEID_REQUIREMENT_FAILED);

		case (IDL.EID_INTERNAL_INTERFACE_ERROR):
			
			String createdStringEID_INTERNAL_INTERFACE_ERROR="Proxy::" + str + "::EID_INTERNAL_INTERFACE_ERROR::" + str2;
			if (Log.ACTIVE) {
				Log.log(Log.ERROR, createdStringEID_INTERNAL_INTERFACE_ERROR);
			}

			throw new IOException(createdStringEID_INTERNAL_INTERFACE_ERROR);

		}

	}

	// ********************Interface Calls********************//

	protected synchronized List<NetworkMInterface> getInterfaces() throws IOException {

		Message msg = sync_request(IDL.FID_GET_INTERFACE, new byte[0]);
		

		if (msg.field1 != 0) {

			checkErr(msg.field1, "getInterfaces()", Deserialize
					.exception(msg.content));

		}
		if (Log.ACTIVE) {
			Log.log(Log.INFO, new String("Ipc::getInterfaces()::succesfull"));
		}
		return Deserialize.desInterfaceList(msg.content);

	}

	// ********************Group Managment Calls********************//

	protected synchronized long createSocket() throws IOException {

		long socketID = 23;

		Message msg = sync_request(IDL.FID_CREATE_SOCKET, new byte[0]);
		
		if (msg.field1 != 0) {

			checkErr(msg.field1, "createSocket()", Deserialize
					.exception(msg.content));

		}

		socketID =  Deserialize.desCreate(msg.content);
	
		if (Log.ACTIVE) {
			Log
					.log(
							Log.INFO,
							new String(
									("Proxy::createSocket(int)::succesfull socketID:" + socketID)));
		}
		return socketID;

	}
	
	

/*
		long socketID = this.createSocket();
		
		
		
		Message msg = sync_request(IDL.FID_CREATE_SOCKET, Marshalling
				.uint32ToByteArray(socketID));
				//.uint32ToByteArray(interafceID)); //TODO:check SocketID
		
		if (msg.field1 != 0) {

			checkErr(msg.field1, "create(int)", Deserialize
					.exception(msg.content));

		}
	
		socketID = Deserialize.desCreate(msg.content);

		if (Log.ACTIVE) {
			Log.log(Log.INFO,
					new String("Proxy::createSocket(int)::succesfull socketID:"
							+ socketID));
		}

		return socketID;

	}*/

	public synchronized void destroy(int socketID) throws IOException {

		Message msg = sync_request(IDL.FID_DELETE_SOCKET, Marshalling
				.uint32ToByteArray(socketID));

		if (msg.field1 != 0) {

			checkErr(msg.field1, "destroy()", Deserialize
					.exception(msg.content));

		}
		if (Log.ACTIVE) {
			Log.log(Log.INFO, new String("Proxy::destroy(int)::socket "
					+ socketID + "destroy"));
		}
	}

	public synchronized void join(int socketID, URI uri) throws IOException {

		Message msg = sync_request(IDL.FID_JOIN, Marshalling.serialize(
				socketID, uri.toString()));

		if (msg.field1 != 0) {

			checkErr(msg.field1, "join()", UnMarshalling.exception(msg.content));

		}
		if (Log.ACTIVE) {
			Log.log(Log.INFO, new String("Proxy::join()::Socket " + socketID
					+ " succesfull joined " + uri.toString()));
		}
	}

	public void leave(int socketID, URI uri) throws IOException {

		Message msg = sync_request(IDL.FID_LEAVE, Serializer.serialize(
				socketID, uri.toString()));

		if (msg.field1 != 0) {

			checkErr(msg.field1, "leave()", Deserialize.exception(msg.content));

		}

		else {
			if (Log.ACTIVE) {
				Log.log(Log.INFO, new String("Proxy::leave()::Socket"
						+ socketID + " succesfull leaved " + uri.toString()));
			}
		}
	}

	// ********************Send and Receive Calls********************//

	public synchronized int createStream(int socketID, String uri)
			throws IOException {

		int sendStream;

		Message msg = sync_request(IDL.FID_CREATE_SEND_STREAM, Marshalling
				.serialize(socketID, uri.toString()));

		if (msg.field1 != 0) {

			checkErr(msg.field1, "createStream()", UnMarshalling
					.exception(msg.content));

		}
		
		sendStream = UnMarshalling.desCreateStream(msg.content);
		if (Log.ACTIVE) {
			Log.log(Log.INFO, new String(
					"Proxy::createStream()::Succesfull create SendStream "
							+ sendStream + " on Socket " + socketID
							+ " for Uri" + uri));
		}
	
		return (int) sendStream;

	}

	// ****************Socket Options***************************//
	public synchronized boolean addSockInterface(int socketID, NetworkMInterface nmi)
	throws IOException {
		boolean success=false;
		Message msg = sync_request(IDL.FID_ADD_SOCK_INTERFACE, Marshalling
				.serialize(socketID, nmi.getIndex(),nmi.getDisplayName(),nmi.getInetAddress(), nmi.getTech()));


		if (msg.field1 != 0) {

			checkErr(msg.field1, "addSockInterface(NetworkMInterface)", Deserialize
					.exception(msg.content));

		}
		
		int zahl = (int) Deserialize.desCreate(msg.content);
		
	
		if(zahl>=0){
			success=true;
		}
		if (Log.ACTIVE) {
			Log
					.log(
							Log.INFO,
							new String(
									("Proxy::addSockInterface::success:" + success)));
		}
		return success;
	}
	//some comment
	public synchronized boolean setSockInterfaces(int socketID, List<Integer> nmis)
	throws IOException {
		boolean success=false;
		Message msg = sync_request(IDL.FID_SET_SOCK_INTERFACE, Marshalling
				.serialize(socketID,nmis));

//
		if (msg.field1 != 0) {

			checkErr(msg.field1, "setSockInterfaces(NetworkMInterface)", Deserialize
					.exception(msg.content));

		}
		
		int zahl = (int) Deserialize.desCreate(msg.content);
		
	
		if(zahl>=0){
			success=true;
		}
		if (Log.ACTIVE) {
			Log
					.log(
							Log.INFO,
							new String(
									("Proxy::setSockInterfaces::success:" + success)));
		}
		return success;
	}
	
	
	public synchronized List<Long> getSockInterfaceIDs(int socketID)
			throws IOException {

		Message msg = sync_request(IDL.FID_GET_SOCK_INTERFACE, Marshalling
				.uint32ToByteArray(socketID));


		if (msg.field1 != 0) {

			checkErr(msg.field1, "getSockInterfaces(socketID)", Deserialize
					.exception(msg.content));

		}
		if (Log.ACTIVE) {
			Log
					.log(
							Log.INFO,
							new String(
									"Proxy::getSockInterfaces(socketID)::get succesfull Interfacelist"));
		}

		return Deserialize.getSockInterfaceIDs(msg.content);
	}
	
	
	
	

	public synchronized boolean delInterfaces(int InterfaceId, int socketId)
			throws IOException {

		Message msg = sync_request(IDL.FID_DEL_SOCK_INTERFACE, Marshalling
				//.uint32ToByteArray(nmi.getIndex()));
				.serialize(socketId,InterfaceId));
				
		boolean success=false;
		if(msg.field1==IDL.EID_NONE){
			success=true;
		}
		
		
		if (msg.field1 != 0) {

			success=false;
			checkErr(msg.field1, "delInterfaces(NetworkMInterface) ",
					Deserialize.exception(msg.content));
			

		}
		if (Log.ACTIVE) {
			Log
					.log(
							Log.INFO,
							new String(
									"Proxy::delInterfaces(NetworkMInterface)::Succesfull delete Interface: "+success));
		}
		return success;
	}
	

	public synchronized boolean setTTL(int hops, int id)
			throws IOException {

		Message msg = sync_request(IDL.FID_SET_TTL, 
				Marshalling.serialize(id,4));
		//		Marshalling.uint32ToByteArray(l));
		boolean success=true;
		if (msg.field1 != 0) {
			success=false;
			checkErr(msg.field1, "setTTL(long l)",
					Deserialize.exception(msg.content));
		}
		if (Log.ACTIVE) {
			Log.log(Log.INFO, new String(
					"Proxy::setTTL(NetworkMInterface [])::succesfull set TTL:"+success));
		}
		return success;
	}

	// ****************Socket Options***************************//

	public synchronized List<GroupSet> groupSet(NetworkMInterface nmi)
			throws IOException {

		Message msg = sync_request(IDL.FID_GROUP_SET, Marshalling
				.uint32ToByteArray(nmi.getIndex()));
		
		if (msg.field1 != 0) {

			checkErr(msg.field1, "groupSet(NetworkMInterface)", Deserialize
					.exception(msg.content));

		}
		if (Log.ACTIVE) {
			Log
					.log(
							Log.INFO,
							new String(
									"Proxy::groupSet(NetworkMInterface)::succesfull receive list"));
		}
		return Deserialize.desGroupSet(msg.content);

	}

	protected ArrayList<URI> neighborSet(NetworkMInterface nmi)
			throws IOException {

		Message msg = sync_request(IDL.FID_NEIGHBOR_SET, Marshalling
				.uint32ToByteArray(nmi.getIndex()));

		if (msg.field1 != 0) {

			checkErr(msg.field1, "neighborSet(NetworkMInterface)", Deserialize
					.exception(msg.content));

		}
		if (Log.ACTIVE) {
			Log
					.log(
							Log.INFO,
							new String(
									"Proxy::groupSet(NetworkMInterface)::succesfull receive list"));
		}
		return Deserialize.desNeighborSet(msg.content);

	}

	protected synchronized ArrayList<URI> childrenSet(NetworkMInterface nmi,
			URI uri) throws IOException {

		Message msg = sync_request(IDL.FID_CHILDREN_SET, Serializer.serialize(
				nmi.getIndex(), uri.toString()));

		if (msg.field1 != 0) {

			checkErr(msg.field1, "childrenSet(NetworkMInterface, URI)",
					Deserialize.exception(msg.content));

		}
		if (Log.ACTIVE) {
			Log
					.log(
							Log.INFO,
							new String(
									"Proxy::childrenSet(NetworkMInterface,URI)::succesfull receive children set"));
		}
		return Deserialize.desChildrenSet(msg.content);

	}
	//this Method returns a List of the parent URIs
	protected synchronized List<URI> parentSet(NetworkMInterface nmi,
			URI uri) throws IOException {

		Message msg = sync_request(IDL.FID_PARENT_SET, Serializer.serialize(nmi
				.getIndex(), uri.toString()));

		if (msg.field1 != 0) {

			checkErr(msg.field1, "parentSet(NetworkMInterface, URI)",
					Deserialize.exception(msg.content));

		}
		if (Log.ACTIVE) {
			Log
					.log(
							Log.INFO,
							new String(
									"Proxy::parentSet(NetworkMInterface,URI)::succesfull receive parentset"));
		}
		return Deserialize.desParentSet(msg.content);

	}

	protected boolean designatedHost(NetworkMInterface nmi, URI uri)
			throws IOException {

		Message msg = sync_request(IDL.FID_DESIGNATED_HOST, Serializer
				.serialize(nmi.getIndex(), uri.toString()));

		if (msg.field1 != 0) {

			checkErr(msg.field1, "designatedHost(NetworkMInterface, URI)",
					Deserialize.exception(msg.content));

		}
		if (Log.ACTIVE) {
			Log
					.log(
							Log.INFO,
							new String(
									"Proxy::designatedHost(NetworkMInterface,URI)::succesfull receive designatedHost"));
		}
		int value = Deserialize.desDesignatedHost(msg.content);

		if (value == 1)
			return true;
		else if (value == 0) {
			return false;
		} else {
			throw new IOException();

		}

	}

	// ****************Synchron Calls**************************//

	private synchronized Message sync_request(int field1, byte[] data) {
	
	//	comModul.ipcSyncBuffer.addRequest(new Message(IDL.SYNC_REQUEST, field1,
	//			requestID++, 0, data));
		
		comModul.ipcSyncBuffer.addRequest(new Message(IDL.SYNC_REQUEST, field1,
						HamcastUtility.getNextNumber(), 0, data));
		
		Message syn_msg;

		do {
			syn_msg = comModul.ipcSyncBuffer.getResponse();

			if (syn_msg == null)
				Thread.yield();

		} while (syn_msg == null);

		return syn_msg;

	}
	
	  public Object clone() throws CloneNotSupportedException
	  {
	    throw new CloneNotSupportedException("cloning's not an option"); 
	    // don't dare you! ;)
	  }
	  


}

