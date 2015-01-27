package libHamCast;

import java.net.URI;
import java.net.URISyntaxException;
import java.util.ArrayList;
import java.util.List;

import multicastApi.GroupSet;
import multicastApi.NetworkMInterface;

class Deserialize {

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
		long sID=UnMarshalling.convertArrayToLong(data, 0, 4);
		return sID;
		
	}

	public static long desCreateStream(byte[] data) {

		return UnMarshalling.convertArrayToLong(data, 0, 2);

	}

	public static List<URI> desParentSet(byte[] data) {

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

	public static List<GroupSet> desGroupSet(byte[] data) {

		String str = "";
		int readpointer = 4;

		int count = UnMarshalling.convertArrayToLong(data, 0, 4);

		List<GroupSet> resultList = new ArrayList<GroupSet>();

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
					resultList.add(new GroupSet(new URI(str), type));
				} catch (URISyntaxException e) {

				}

			}
		}

		return resultList;
	}

	public static List<NetworkMInterface> desInterfaceList(byte[] data) {

		int readPointer = 0;
		int length = 0;
		@SuppressWarnings("unused")
		int socketID = 0;
		long index;
		String modul;
		String adress;
		String tech;

		
		
		long amount=UnMarshalling.convertArrayToLong(data, readPointer, 4);
		readPointer += 4;

		//List to fill with existing networks
		List<NetworkMInterface> networkList=new ArrayList<NetworkMInterface>();
		
		for(long i=1;i<=amount;i++){
		// index
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

		networkList.add(netMInter);
		}
		return networkList;
	

	}
	
	//returns only socket infaces ids
	public static List<Long> getSockInterfaceIDs(byte[] data){
		int readPointer = 0;
	
		
		long amount=UnMarshalling.convertArrayToLong(data, readPointer, 4);
		readPointer += 4;

		
		long id;
		List<Long> ids=new ArrayList<Long>();
		for(long i=0;i<amount;i++){
			// index
			id = UnMarshalling.convertArrayToLong(data, readPointer, 4);
			readPointer += 4;

			ids.add(id);
		}
		
		return ids;
	}
	

}
