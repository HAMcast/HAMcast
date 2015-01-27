package libHamCast;

import java.net.URI;
import java.net.URISyntaxException;

class Message {

	protected int messageTyp = 0;
	protected int field1 = 0;
	protected long field2 = 0;
	protected long field3 = 0;
	public long contentSize = 0;
	protected byte[] content;

	protected int ipcMsgSize = 0;

	protected Message(int messageTyp, int field1, long field2, long field3,
			byte[] content) {

		this.messageTyp = messageTyp;
		this.field1 = field1;
		this.field2 = field2;
		this.field3 = field3;
		if (content != null) {
			this.contentSize = content.length;
			this.content = content;
			this.ipcMsgSize = content.length + 16;
		}

	}

	protected byte[] toByte() {

		byte[] msg = new byte[16 + content.length];

		System.arraycopy(Marshalling.uint16ToByteArray((int) messageTyp), 0,
				msg, 0, 2);
		System.arraycopy(Marshalling.uint16ToByteArray((int) field1), 0, msg,
				2, 2);
		System.arraycopy(Marshalling.uint32ToByteArray((int) field2), 0, msg,
				4, 4);
		System.arraycopy(Marshalling.uint32ToByteArray((int) field3), 0, msg,
				8, 4);
		System.arraycopy(Marshalling.uint32ToByteArray((int) contentSize), 0,
				msg, 12, 4);
		if (contentSize != 0) {
			System.arraycopy(content, 0, msg, 16, content.length);
		}

		return msg;
	}

	protected URI getUri() {

		URI uri;

		// parsing uri
		long adresslen = UnMarshalling.convertArrayToLong(content, 0, 4);
		try {
			uri = new URI(new String(content, 4, (int) adresslen));
		} catch (URISyntaxException e) {
			Log.log(Log.FATAL,
					"Message::getUri()::Keine gueltige Uri empfangen");
			return null;
		}

		return uri;
	}

	protected byte[] getPacket() {

		long adresslen = UnMarshalling.convertArrayToLong(content, 0, 4);
		long cs = UnMarshalling.convertArrayToLong(content,
				4 + (int) adresslen, 4);
		byte[] packet = new byte[(int) cs];
		System.arraycopy(content, 8 + (int) adresslen, packet, 0, (int) cs);
		return packet;

	}

}
