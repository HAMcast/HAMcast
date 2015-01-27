package libHamCast;

class Serializer {

	public static byte[] serialize(int intvalue, String stringValue) {

		// marshalling
		byte[] first = Marshalling.uint32ToByteArray(intvalue);
		byte[] second = Marshalling.stringToByteArray(stringValue);
		// serialise
		byte[] msg = new byte[first.length + second.length];
		System.arraycopy(first, 0, msg, 0, first.length);
		System.arraycopy(second, 0, msg, first.length, second.length);
		// serialise data with extern data representation
		return msg;

	}

}
