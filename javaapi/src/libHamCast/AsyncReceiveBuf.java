package libHamCast;

import java.util.Queue;

import multicastApi.MessageContent;
// used in Communication - class.
class AsyncReceiveBuf {

	protected void add(Message msg) {

		// takes reference from field 2
		MulticastSocket socket = MulticastSocket.socketList.get((int) msg.field2);
	
		if (socket != null) {
			
			// alte version da hier schon in der API nach der Uri aufgeteilt
			// worden ist
			// Queue<byte []>
			// buf=socket.receiveBuffer.get(msg.getUri().toString());
		Queue<MessageContent> buf = socket.receiveBlockQueue;
			
			if (buf!= null) {
				
		//		byte[] packet = msg.getPacket();
				MessageContent packet=new MessageContent(msg.getUri(), msg.getPacket());
				if (socket.receiveBufferSize > packet.asBytes().length) {

					socket.writeLock.lock();
					socket.receiveBufferSize = socket.receiveBufferSize
							- packet.asBytes().length;

					socket.writeLock.unlock();
				
					buf.add(packet);
					
				//	 socket.receivelock.lock();
				//	socket.receiveCondition.signal();
				//	socket.receivelock.unlock();

				} else {
					// socket.writeLock.unlock();
					// System.out.println("packet dropped");
					Log
							.log(Log.FATAL,
									"AsyncReceiveBuf::add()::packed dropped");

				}

			}

		}else {
			Log
			.log(Log.FATAL,
					"AsyncReceiveBuf::add()::some Socket could not be recognised");
		}
	}

}
