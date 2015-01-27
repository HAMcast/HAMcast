package libHamCast;

import java.net.URI;
import java.net.URISyntaxException;
import java.util.ArrayList;

import multicastApi.GroupSet;
import multicastApi.NetworkMInterface;

class UnMarshalling {

	public static int unsignedByteToInt(byte b) {
		return (int) b & 0xFF;
	}

	public static String convertArrayToString(byte[] stream, long start,
			long size) {
		String text = "";
		for (long i = start; i < (start + size); i++) {
			text += (char) stream[(int) i];
		}
		return text;
	}

	public static int convertArrayToLong(byte[] b, int start, int size) {
		int value = 0;

		
		for (int i = start; i < (start + size); i++) {

			value += unsignedByteToInt(b[i]) << (8 * (i - start));

		}
	//	System.out.println("value " +value);
		return value;
	}

	public static ArrayList<URI> desNeighborSet(byte[] data) {

		String str = "";
		int readpointer = 4;

		int count = UnMarshalling.convertArrayToLong(data, 0, 4);

		ArrayList<URI> arrayList = new ArrayList<URI>();

		if (count != 0) {

			for (int i = 0; i < count; i++) {

				int length = UnMarshalling.convertArrayToLong(data,
						readpointer, 4);
				readpointer += 4;

				str = UnMarshalling.convertArrayToString(data, readpointer,
						length);
				readpointer += length;

				try {
					arrayList.add(new URI(str));
				} catch (URISyntaxException e) {

				}

			}
		}

		return arrayList;
	}

	public static String exception(byte[] data) {

		String str = "";

		int count = UnMarshalling.convertArrayToLong(data, 0, 4);

		if (count != 0) {

			for (int i = 4; i < count; i++) {

				str = str + (char) data[i];

			}
		}

		return str;
	}

	public static ArrayList<URI> desChildrenSet(byte[] data) {

		String str = "";
		int readpointer = 4;

		int count = UnMarshalling.convertArrayToLong(data, 0, 4);

		ArrayList<URI> arrayList = new ArrayList<URI>();

		if (count != 0) {

			for (int i = 0; i < count; i++) {

				int length = UnMarshalling.convertArrayToLong(data,
						readpointer, 4);
				readpointer += 4;

				str = UnMarshalling.convertArrayToString(data, readpointer,
						length);
				readpointer += length;

				try {
					arrayList.add(new URI(str));
				} catch (URISyntaxException e) {

				}

			}
		}

		return arrayList;
	}

	public static long desCreate(byte[] data) {

		return UnMarshalling.convertArrayToLong(data, 0, 4);

	}

	public static int desCreateStream(byte[] data) {

		return UnMarshalling.convertArrayToLong(data, 0, 2);

	}

	public static ArrayList<URI> desParentSet(byte[] data) {

		String str = "";
		int readpointer = 4;

		int count = UnMarshalling.convertArrayToLong(data, 0, 4);

		ArrayList<URI> arrayList = new ArrayList<URI>();

		if (count != 0) {

			for (int i = 0; i < count; i++) {

				int length = UnMarshalling.convertArrayToLong(data,
						readpointer, 4);
				readpointer += 4;

				str = UnMarshalling.convertArrayToString(data, readpointer,
						length);
				readpointer += length;

				try {
					arrayList.add(new URI(str));
				} catch (URISyntaxException e) {

				}

			}
		}

		return arrayList;
	}

	public static int desDesignatedHost(byte[] data) {

		int value = data[0];

		return value;
	}

	public static ArrayList<GroupSet> desGroupSet(byte[] data) {

		String str = "";
		int readpointer = 4;

		int count = UnMarshalling.convertArrayToLong(data, 0, 4);

		ArrayList<GroupSet> arrayList = new ArrayList<GroupSet>();

		if (count != 0) {

			for (int i = 0; i < count; i++) {

				int length = UnMarshalling.convertArrayToLong(data,
						readpointer, 4);
				readpointer += 4;

				str = UnMarshalling.convertArrayToString(data, readpointer,
						length);
				readpointer += length;

				int type = UnMarshalling.convertArrayToLong(data, readpointer,
						4);
				readpointer += 4;

				try {
					arrayList.add(new GroupSet(new URI(str), type));
				} catch (URISyntaxException e) {

				}

			}
		}

		return arrayList;
	}

	public static synchronized NetworkMInterface desInterfaceList(byte[] data) {

		int readPointer = 0;
		int length = 0;
		@SuppressWarnings("unused")
		int socketID = 0;
		long index;
		String modul;
		String adress;
		String tech;

		// Noch unklar was der erste Wert bedeutet! Anzahl an Übergabeoarameter?
		UnMarshalling.convertArrayToString(data, readPointer, 4);
		readPointer += 4;

		index = UnMarshalling.convertArrayToLong(data, readPointer, 4);
		readPointer += 4;

		// modul
		length = UnMarshalling.convertArrayToLong(data, readPointer, 4);
		readPointer += 4;
		modul = UnMarshalling.convertArrayToString(data, readPointer, length);
		readPointer += length;

		// adresse
		length = UnMarshalling.convertArrayToLong(data, readPointer, 4);
		readPointer += 4;
		adress = UnMarshalling.convertArrayToString(data, readPointer, length);
		readPointer += length;

		// technologie
		length = UnMarshalling.convertArrayToLong(data, readPointer, 4);
		readPointer += 4;
		tech = UnMarshalling.convertArrayToString(data, readPointer, length);
		readPointer += length;

		NetworkMInterface netMInter = new NetworkMInterface((int) index, modul,
				adress, tech);



		return netMInter;

	}

}
