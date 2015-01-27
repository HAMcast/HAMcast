package libHamCast;

import java.util.concurrent.ConcurrentLinkedQueue;
import java.util.concurrent.locks.Condition;

import java.util.concurrent.locks.ReentrantLock;

class IpcSenderPacketAssembly extends Thread {

	private MiddlewareStream stream;

	public static final ReentrantLock lock = new ReentrantLock();
	public static Condition condition = lock.newCondition();

	public static ConcurrentLinkedQueue<Message> queue;

	public static int stocksize = 0;

	private Communication comModul;

	private byte[] sobuffer = new byte[Parameter.ASYN_SNDBUF];

	public IpcSenderPacketAssembly(Communication comModul) {

		this.stream = comModul.middlewareStream;
		this.comModul = comModul;

		setDaemon(true);

		queue = new ConcurrentLinkedQueue<Message>();

	}

	public static void add(Message msg) {

		queue.add(msg);
		lock.lock();
		condition.signal();
		lock.unlock();

	}

	/*
	 * public static void write(Message msg) {
	 * 
	 * stream.write(msg.toByte()); }
	 */

	@Override
	public void run() {

		while (true) {

			if (!comModul.ipcSyncBuffer.isEmpty()) {
				stream.write(comModul.ipcSyncBuffer.getRequest().toByte());
			}

			else {

				lock.lock();
				Message msg = queue.poll();

				if (msg == null) {

					/*
					 * try { condition.await(); } catch (InterruptedException e)
					 * { // TODO Auto-generated catch block e.printStackTrace();
					 * }
					 */
					lock.unlock();

				} else {

					lock.unlock();
					int size = 0;

					do {
						byte[] tmp = msg.toByte();

						System
								.arraycopy(tmp, 0, sobuffer, size,
										msg.ipcMsgSize);
						size += msg.ipcMsgSize;

						msg = queue.poll();

						if (msg == null)
							break;

					} while ((size + msg.ipcMsgSize) < Parameter.ASYN_SNDBUF);

					if (size != 0) {
						stream.write(sobuffer, 0, size);
					}
				}

			}

		}

	}

}
