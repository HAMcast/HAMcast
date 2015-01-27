package libHamCast;

import java.util.concurrent.locks.Condition;
import java.util.concurrent.locks.Lock;

class HelperAck extends Thread {

	private Condition condition;
	private Lock lock;

	protected static int socketID;
	protected static long streamID;
	protected static int ack;

	public HelperAck(Condition condition, Lock lock) {

		this.condition = condition;
		this.lock = lock;

		setDaemon(true);

	}

	@Override
	public void run() {

		while (true) {

			lock.lock();

			try {
				condition.await();
			} catch (InterruptedException e) {

			}
			MulticastSocket mSocket = MulticastSocket.socketList
					.get(socketID);
	
			mSocket.socksSendMap_StreamID.get(Integer.valueOf((int)streamID)).ack((int) ack);

			lock.unlock();

		}

	}

}
