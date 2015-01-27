package libHamCast;

//Tags for the Communication inside the packages which go to the middleware.
class IDL {

	// Function ID
	protected static final int FID_CREATE_SOCKET = 0x0001;
	protected static final int FID_DELETE_SOCKET = 0x0002;
	protected static final int FID_CREATE_SEND_STREAM = 0x0003;
	protected static final int FID_JOIN = 0x0004;
	protected static final int FID_LEAVE = 0x0005;
	protected static final int FID_SET_TTL = 0x0006;
	protected static final int FID_GET_SOCK_INTERFACE = 0x0007;
	protected static final int FID_ADD_SOCK_INTERFACE = 0x0008;
	protected static final int FID_DEL_SOCK_INTERFACE = 0x0009;
	protected static final int FID_SET_SOCK_INTERFACE = 0x000A;

	// Service Calls
	protected static final int FID_GET_INTERFACE = 0x0100;
	protected static final int FID_GROUP_SET = 0x0101;
	protected static final int FID_NEIGHBOR_SET = 0x0102;
	protected static final int FID_PARENT_SET = 0x0103;
	protected static final int FID_CHILDREN_SET = 0x0104;
	protected static final int FID_DESIGNATED_HOST = 0x0105;
	protected static final int SYNC_REQUEST = 0x00;
	protected static final int SYNC_RESPONSE = 0x01;
	protected static final int ASYNC_EVENT = 0x02;
	protected static final int ASYNC_SEND = 0x03;
	protected static final int ASYNC_RECV = 0x04;
	protected static final int CUMULATIVE_ACK = 0x05;
	protected static final int RETRANSMIT = 0x06;

	// Exeptions
	protected static final int EID_NONE = 0x0000;
	protected static final int EID_REQUIREMENT_FAILED = 0x0002;
	protected static final int EID_INTERNAL_INTERFACE_ERROR = 0x003;

}
