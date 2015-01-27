package libHamCast;

import java.util.ArrayList;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ConcurrentMap;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantReadWriteLock;

class Sendbuf {

	public ConcurrentMap<Integer, Message> sendBuf;

	private static final ReentrantReadWriteLock lock = new ReentrantReadWriteLock();
	private static final Lock writeLock = lock.writeLock();

	protected String uri = "";

	protected int streamID = 0;
	protected int sendPointer = 0;
	protected int socketID = 0;
	protected int size = 0;
	protected int lastack = 0;
	protected int biggestAck = 0;
	MulticastSocket mSocket;
	protected int sendBufferSize;

	public Sendbuf(String uri, int streamID, int socketID,
			MulticastSocket mSocket) {

		this.uri = uri;
		this.streamID = streamID;
		this.sendBuf = new ConcurrentHashMap<Integer, Message>();
		sendPointer = 0;
		this.socketID = socketID;
		this.mSocket = mSocket;
		this.sendBufferSize = Parameter.ASYN_SNDBUF;

	}

	public void flush() {

		while (sendBuf.size() > 1)
			;

	}

	public void ret(int ret) {

		IpcSender.add(sendBuf.get(ret));

	}

	protected void ack(int ack) {

		// Log.datei("Ack\t" + ack + "\tSendbuffersize\t" + size);

		// TODO: If Abfrage ist nur, weil im IPC Modul ein Bug ist. In
		// Grenzsituationen bekomme ich ack die zum Teil kleiner sind als schon
		// abgehandelte
		if (ack > lastack) {

			for (int i = lastack; i < ack; i++) {
				// Thread Sicher
				Message msg = sendBuf.remove(i);

				if (msg != null) {
					writeLock.lock();
					// size--;
					sendBufferSize += msg.ipcMsgSize;
					writeLock.unlock();
				}

			}

			lastack = ack;

		}

		mSocket.sendlock.lock();
		mSocket.sendCondition.signal();
		mSocket.sendlock.unlock();
	}

	protected boolean add(byte[] data) {
		
		
		
		
		
		
		Message msg = new Message(IDL.ASYNC_SEND, streamID, socketID,
				sendPointer, data);

		if (sendBufferSize > msg.ipcMsgSize) {

			writeLock.lock();

			sendBuf.put(sendPointer, msg);
			//IpcSender.add(msg);
		//	if(!IpcSender.write(msg))return false;
			boolean worked=IpcSender.write(msg);
			sendPointer++;
			sendBufferSize -= msg.ipcMsgSize;
			writeLock.unlock();

			return worked;

		} else {
			// Log.log(Log.WARN, "Sendbuf::add()::full::" + sendPointer);
			System.out.println("full");

			return false;

		}

	}

}
