package libHamCast;

class Parameter {

	protected static final long m_magic_number = 0xDEADC0DE;
	protected static final long m_major_version = 0x0;
	protected static final long m_minor_version = 0x6;

	// protected static int RECEIVE_BUFFER_SIZE = 16*1024*1024; //16 Megabyte

	// Buffer für den SendBuf pro Socket
	public static int ASYN_SNDBUF = 16 * 1024 * 1024;
	public static int ASYN_RCVBUF = 10 * 1024 * 1024;

	public static int SO_SNDBUF = 80000;
	public static int SO_RCVBUF = 200000;
	public static boolean TCP_NODELAY = false;
	
	
	//initialized after getting connection to middleware
	private static long MAX_MESSAGE_SIZE=0;
	protected static void setMaxMessage(long size){
		MAX_MESSAGE_SIZE=size;
	}
	/* MaxMessageSize is initialized with the first connection to middleware. Before Initialisation the MaxMessageSize is "0".
	 * @return Max Size in byte which hamcast-middleware can send.  
	 */
	public static long getMaxMessageSize(){
		return MAX_MESSAGE_SIZE;
	}
}
