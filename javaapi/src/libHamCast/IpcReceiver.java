package libHamCast;

import java.awt.EventQueue;
import java.util.concurrent.locks.Condition;
import java.util.concurrent.locks.ReentrantLock;

class IpcReceiver extends Thread  {

	private MiddlewareStream ipcStream;
	private Communication comModul;

	private Condition conditionAck;
	private ReentrantLock lockAck;
	HelperAck helperAck;

	public int countreceive = 0;
	private Runnable r;
	boolean stopThread=false;
	


	public static int unsignedByteToInt(byte b) {
		return (int) b & 0xFF;
	}

	public int convertArrayToLong(byte[] b, int start, int size) {
		int value = 0;
		for (int i = start; i < start + size; i++) {

			value += unsignedByteToInt(b[i]) << (8 * (i - start));

		}
		return value;
	}

	
	public IpcReceiver(Communication comModul) {

		this.ipcStream = comModul.middlewareStream;

		this.lockAck = new ReentrantLock();
		this.conditionAck = lockAck.newCondition();

		helperAck = new HelperAck(conditionAck, lockAck);
		helperAck.start();

		this.comModul = comModul;

		setDaemon(true);

	}
	
	
	public IpcReceiver(Communication comModul, Runnable exceptionHandler){

		this.ipcStream = comModul.middlewareStream;

		this.lockAck = new ReentrantLock();
		this.conditionAck = lockAck.newCondition();

		helperAck = new HelperAck(conditionAck, lockAck);
		helperAck.start();

		this.comModul = comModul;
		this.r=exceptionHandler;
		setDaemon(true);

	}

	public void run() {

		int messageType = 0;
		int field1 = 0;
		long field2 = 0;
		long field3 = 0;
		int cs = -1;
		byte[] data = null;

		byte[] readArray = new byte[Parameter.SO_RCVBUF];
		int remains = 0;
		int head = 0;
		int tail = 0;
		boolean empty = false;
		boolean lastHead = false;
		int temp = 0;
		long timeNow = 0;


		while (!stopThread) {

			try{
			if (remains == 0 || empty) {

				if (head == tail) {
					tail = 0;
					
				} else {
					
					int remainSize = tail - head;
				//	if(remainSize<0)  {this.interrupt();}else{
					System.arraycopy(readArray, head, readArray, 0, remainSize);
					tail = remainSize;
				//	}
				}
				timeNow = System.currentTimeMillis() - timeNow;

				head = 0;
				int size = readArray.length - tail;
				
				remains = ipcStream.readMax(readArray, tail, size);
		
				if (Log.ACTIVE) {
					Log.log(Log.TRACE, "IpcReceiver::Begin " + tail + "size "
							+ size);
					Log.log(Log.TRACE, "IpcReceiver::Thread::need " + timeNow
							+ "ms to save " + temp + " Byte");
					Log.log(Log.DEBUG, "IpcReceiver::Thread::read " + remains
							+ " Byte from TCP Buffer");
				}
				tail = remains + tail;
				remains = tail;
				temp = remains;
				empty = false;


				if (Log.ACTIVE) {
					Log.log(Log.DEBUG,
							"IpcReceiver::Thread::readArray contains "
									+ remains + " Byte");
					Log.log(Log.TRACE,
							"IpcReceiver::Thread::now parsing the Array");
				}

				timeNow = System.currentTimeMillis();
			}

			if (!lastHead) {
				if (remains >= 16) {

					messageType = convertArrayToLong(readArray, head, 2);
					head += 2;
					field1 = convertArrayToLong(readArray, head, 2);
					head += 2;
					field2 = convertArrayToLong(readArray, head, 4);
					head += 4;
					field3 = convertArrayToLong(readArray, head, 4);
					head += 4;
					cs = convertArrayToLong(readArray, head, 4);

					head += 4;

					remains -= 16;
	
					// Falls der Buffer leer steht am Anfang des neuen Array
					// erst das Packet!
					lastHead = true;
				
				} else {

					empty = true;

				}
			}

			if (remains >= cs) {

				if (cs > 0) {
					// Erzeugung eine neuen Objekts
					data = new byte[cs];
					System.arraycopy(readArray, head, data, 0, cs);
					head += cs;
					remains -= cs;
			
				}

				lastHead = false;
				
				// Async Message
				if (messageType == IDL.ASYNC_RECV) {
				
					comModul.ipcAsyncReceiveBuf.add(new Message(messageType,
							field1, field2, field3, data));

					if (Log.ACTIVE) {
						Log.log(Log.DEBUG,
								"IpcReceiver::Thread::receive async msg");
					}

				}

				// Sync Message
				else if (messageType == IDL.SYNC_RESPONSE) {

					comModul.ipcSyncBuffer.addResponse(new Message(messageType,
							field1, field2, field3, data));
					if (Log.ACTIVE) {
						Log.log(Log.DEBUG,
								"IpcReceiver::Thread::receive sync message");
					}

				}

				// Cumulative Acknowledge
				else if (messageType == IDL.CUMULATIVE_ACK) {

					// Holt sich aus der globalen liste das socket

					// Critical Section
					lockAck.lock();

					HelperAck.socketID = (int)field2;
					HelperAck.streamID = field1;
					HelperAck.ack = (int)field3;

					conditionAck.signal();

					lockAck.unlock();
					if (Log.ACTIVE) {
						Log.log(Log.DEBUG, "IpcReceiver::Thread::receive ACK: "
								+ field3 + "\tSocked: " + field2
								+ "\tStreamID: " + field1);
					}

				}

				// RETRANSMIT
				else if (messageType == IDL.RETRANSMIT) {

					if (Log.ACTIVE) {
						Log.log(Log.DEBUG,
								"IpcReceiver::Thread::receive retransmit");
					}

					// Holt sich aus der globalen liste das socket
					MulticastSocket mSocket = MulticastSocket.socketList
								.get((int) field2);
			
					mSocket.socksSendMap_StreamID.get((int) field1).ret(
							(int) field3);

				} else {
					if (Log.ACTIVE) {
						Log.log(Log.FATAL,
								"IPC Receiver Thread::run()::receive invalid message "
										+ messageType);
					}
				}

			}

			else {

				empty = true;

			}
		}catch(Exception e){
			e.printStackTrace();
			if (!stopThread && r==null) {
		
				stopThread=true;
					EventQueue.invokeLater(
							
						r=HamcastUtility.getExceptionHandler()
						
					);
					this.interrupt();
			
				
			} else if(!stopThread){
				stopThread=true;
				EventQueue.invokeLater(r);

				
				this.interrupt();
				
			}else{
				stopThread=true;
				this.interrupt();
			}
		}
		}
	
	}

	


	}
