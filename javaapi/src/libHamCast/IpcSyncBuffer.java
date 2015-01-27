package libHamCast;

import java.util.LinkedList;
import java.util.Queue;

class IpcSyncBuffer {

	private Queue<Message> ipcSyncRequestBuffer;
	private Queue<Message> ipcSyncResponseBuffer;

	public IpcSyncBuffer() {
		

		
		ipcSyncRequestBuffer = new LinkedList<Message>();
		ipcSyncResponseBuffer = new LinkedList<Message>();

	}

	protected void addResponse(Message msg) {

		ipcSyncResponseBuffer.add(msg);

	}

	protected Message getRequest() {

		return ipcSyncRequestBuffer.poll();

	}

	protected void addRequest(Message msg) {

		// ipcSyncRequestBuffer.add(msg);
		IpcSender.write(msg);

	}

	protected Message getResponse() {

		return ipcSyncResponseBuffer.poll();

	}

	protected boolean isEmpty() {

		if (ipcSyncRequestBuffer.isEmpty())
			return true;
		return false;

	}

}
