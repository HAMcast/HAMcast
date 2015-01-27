package libHamCast;

import java.awt.EventQueue;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

import java.net.Socket;

class MiddlewareStream {

	private Socket middlewareSocket = null;
	private OutputStream out = null;
	private InputStream in = null;
	protected static int mms;
	Runnable r;

	public MiddlewareStream(String adress, int port, long magicNumber,
			long majorNumber, long minorNumber) throws IOException {

		middlewareSocket = new Socket(adress, port);

		middlewareSocket.setReceiveBufferSize(Parameter.SO_RCVBUF);
		Log.log(Log.INFO, "TCP Receivebuffer: "
				+ middlewareSocket.getReceiveBufferSize());

		middlewareSocket.setSendBufferSize(Parameter.SO_SNDBUF);
		Log.log(Log.INFO, "TCP Sendbuffer: "
				+ middlewareSocket.getSendBufferSize());

		middlewareSocket.setTcpNoDelay(Parameter.TCP_NODELAY);
		Log.log(Log.INFO, "TCP NoDelay:" + Parameter.TCP_NODELAY);

		this.out = middlewareSocket.getOutputStream();
		this.in = middlewareSocket.getInputStream();

		write(Marshalling.uint32ToByteArray(Parameter.m_magic_number));
		write(Marshalling.uint32ToByteArray(Parameter.m_major_version));
		write(Marshalling.uint32ToByteArray(Parameter.m_minor_version));

		if (read() != 1) {
			Log.log(Log.FATAL, "Your API-Version is incompatible with HAMcast");
			throw new IOException("incompatible Api-Version");

		}
		//int bla=read();
		byte array[] = new byte[4];
		readMax(array, 0, 4);
		long max_msg_size= UnMarshalling.convertArrayToLong(array, 0, 4);
		Parameter.setMaxMessage(max_msg_size);


	}

	public int read() {

		int value = -1;
		try {
			value = in.read();
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
			return value;
		}
		return value;

	}

	public int readMax(byte[] array, int pos, int size) {

		int count = 0;

		try {

			count = in.read(array, pos, size);
	
		} catch (IOException e) {
			e.printStackTrace();

		}
		
		return count;

		

	}

	public boolean write(byte[] data) {

		try {
			out.write(data);
		
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
			return false;
		}
		return true;

	}

	public boolean write(byte[] data, int start, int len) {

		try {
			out.write(data, start, len);
			
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
			return false;
		}
		return true;

	}

	class checkSockets extends Thread {
		public void run() {

			try {
				Thread.sleep(5000);

			} catch (InterruptedException e) {
				if (r == null) {
					EventQueue.invokeLater(new Runnable() {

						@Override
						public void run() {
							try {
								if (!middlewareSocket.getInetAddress()
										.isReachable(5000)) {
									throw new RuntimeException(
											"Couldn't reach middleware - is it Running?");
								}
							} catch (IOException e) {

							}

						}

					});
				} else {
					EventQueue.invokeLater(r);
				}
			}
		}

	}
}
