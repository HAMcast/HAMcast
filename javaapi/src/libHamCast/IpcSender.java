package libHamCast;

import java.util.concurrent.ConcurrentLinkedQueue;
import java.util.concurrent.locks.Condition;

import java.util.concurrent.locks.ReentrantLock;
/**handles sendings between middleware and this library (local) */
class IpcSender extends Thread {

	private static MiddlewareStream stream;

	public static final ReentrantLock lock = new ReentrantLock();
	public static Condition condition = lock.newCondition();

	public static ConcurrentLinkedQueue<Message> queue;

	public static int stocksize = 0;

	private Communication comModul;

	public IpcSender(Communication comModul) {

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

	public static boolean write(Message msg) {

		return stream.write(msg.toByte());
	}

	@Override
	public void run() {

		while (true) {

			if (!comModul.ipcSyncBuffer.isEmpty()) {
				stream.write(comModul.ipcSyncBuffer.getRequest().toByte());
				
			}

			/*else {

				
				 * lock.lock(); Message msg = queue.poll();
				 * 
				 * if (msg == null) {
				 * 
				 * 
				 * try { condition.await(); } catch (InterruptedException e) {
				 * // TODO Auto-generated catch block e.printStackTrace(); }
				 * lock.unlock();
				 * 
				 * } else{
				 * 
				 * lock.unlock(); //stream.write(msg.toByte());
				 * 
				 * 
				 * }
				 

			}*/

		}

	}

}
