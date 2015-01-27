package libHamCast;

import java.util.concurrent.locks.Condition;
import java.util.concurrent.locks.Lock;

class HelperRetr extends Thread {

	Condition condition;
	Lock lock;

	public long retr = 0;

	// public AsyncSendBuf ipcAsyncSendBuf;

	public HelperRetr(Condition condition, Lock lock) {

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

				break;
			}

			lock.unlock();

			// ipcAsyncSendBuf.retransmit(retr);

		}

	}

}
