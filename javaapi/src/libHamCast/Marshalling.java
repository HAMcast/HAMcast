package libHamCast;


import java.util.List;


class Marshalling {
	/*
	 * Handles the changing from object to network order.
	 * */
	public synchronized static byte[] uint32ToByteArray(long value) {
		/*
		 * value= f e d c most significant byte:f last significant byte:c little
		 * Endian= LSB>>lowest address and MSB(lastByte)>>highest address
		 */
		byte[] int_byte = new byte[4];
		// int_byte[0] = c
		int_byte[0] = (byte) (value);
		// int_byte[0] = d
		int_byte[1] = (byte) (value >>> 8);
		// int_byte[0] = e
		int_byte[2] = (byte) (value >>> 16);
		// int_byte[0] = f
		int_byte[3] = (byte) (value >>> 24);
		// int_byte[c,d,e,f]
		return int_byte;
	}
	


	public synchronized static byte[] uint16ToByteArray(int value) {

		byte[] int_byte = new byte[2];
		int_byte[0] = (byte) (value);
		int_byte[1] = (byte) (value >>> 8);
		return int_byte;
	}

	public synchronized static byte[] stringToByteArray(String string) {
		// length of string as uint32 (little endian)
		byte[] strlength = uint32ToByteArray(string.length());
		// size of marshalling string
		byte[] msg = new byte[strlength.length + string.length()];
		// concat string length and string
		System.arraycopy(strlength, 0, msg, 0, strlength.length);
		System.arraycopy(string.getBytes(), 0, msg, strlength.length, string
				.length());

		return msg;
	}

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
	public static byte[] serialize(int intvalue, int intvalue2) {

		// marshalling
		byte[] first = Marshalling.uint32ToByteArray(intvalue);
		byte[] second = Marshalling.uint32ToByteArray(intvalue2);
		// serialise
		byte[] msg = new byte[first.length + second.length];
		System.arraycopy(first, 0, msg, 0, first.length);
		System.arraycopy(second, 0, msg, first.length, second.length);
		// serialise data with extern data representation
		return msg;

	}


	
	public static byte[] serialize(int socketID, List<Integer> nmis){
		byte[] sID = Marshalling.uint32ToByteArray(socketID);
		byte[] anzahlIs=Marshalling.uint32ToByteArray(nmis.size());
		byte[]nmiList=null;
		
		for(Integer nmi:nmis){
			//serializing the first interface data
			byte[] nmiIndex= Marshalling.uint32ToByteArray(nmi);
			System.out.println("Marshalling nmiID: "+nmi);
			int anzahl=nmiIndex.length;
			byte[]msg=new byte[anzahl];
			System.arraycopy(nmiIndex, 0, msg, 0, nmiIndex.length);
			
			
			
			if(nmiList==null){
				nmiList=new byte[nmiIndex.length];
				System.arraycopy(msg, 0, nmiList, 0, msg.length);
			}
			else{
				//create temporary bytearray for old nmis plus the next one
				byte[] newNmiList=new byte[nmiList.length+msg.length];
				System.arraycopy(nmiList, 0, newNmiList, 0, nmiList.length);
				System.arraycopy(msg, 0, newNmiList, nmiList.length, msg.length);
				
				//getting them to the final list
				
				nmiList=newNmiList;
				
				
			}
			 

		}
		byte[] finalMessage=null;
		if(nmiList!=null){
		finalMessage=new byte[sID.length+anzahlIs.length+nmiList.length];
		
		System.arraycopy(sID, 0, finalMessage, 0, sID.length);
		System.arraycopy(anzahlIs, 0, finalMessage, sID.length, anzahlIs.length);
		System.arraycopy(nmiList, 0, finalMessage, sID.length+anzahlIs.length, nmiList.length);
		
		}else{
			finalMessage=new byte[sID.length+anzahlIs.length];
			
			System.arraycopy(sID, 0, finalMessage, 0, sID.length);
			System.arraycopy(anzahlIs, 0, finalMessage, sID.length, anzahlIs.length);
			
		}
		

		return finalMessage;
	}
	
	public static byte[] serialize(long socketID, int nmiID, String nmiName, String nmiAddress, String nmiTech ) {
		byte[] first = Marshalling.uint32ToByteArray(socketID);
		byte[] second= Marshalling.uint32ToByteArray(nmiID);
		byte[] third=Marshalling.stringToByteArray(nmiName);
		byte[] fourth=Marshalling.stringToByteArray(nmiAddress);

		byte[] fifth=Marshalling.stringToByteArray(nmiTech);

		int amount=first.length+second.length+third.length+fourth.length+fifth.length;
		byte[]msg=new byte[amount];
		System.arraycopy(first, 0, msg,0,first.length);
		System.arraycopy(second, 0, msg, first.length, second.length);
		System.arraycopy(third, 0, msg, (first.length)+(second.length), third.length);
		System.arraycopy(fourth, 0, msg, first.length+second.length+third.length, fourth.length);
		System.arraycopy(fifth, 0, msg, first.length+second.length+third.length+fourth.length, fifth.length);
		
		
		
		return msg;
	}


	

}
