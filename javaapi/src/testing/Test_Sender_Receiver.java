package testing;

import java.io.IOException;
import java.net.URI;
import java.net.URISyntaxException;
import java.util.Iterator;

import libHamCast.HamcastUtility;
import libHamCast.Log;
import libHamCast.MulticastSocket;
import multicastApi.GroupSet;
import multicastApi.NetworkMInterface;
import static libHamCast.MulticastSocket.*;

public class Test_Sender_Receiver {

	public static void main(String[] args) throws IOException,
			URISyntaxException {

		Log.logLevel = Log.FATAL;
		NetworkMInterface nmi = null;
		GroupSet groupSet = null;

		Iterator<NetworkMInterface> netMInter = HamcastUtility.getAllInterfaces().iterator();

		while (netMInter.hasNext()) {

			nmi = netMInter.next();

			System.out.println("index: " + nmi.getIndex());
			System.out.println("load: " + nmi.getDisplayName());
			System.out.println("adress: " + nmi.getInetAddress());
			System.out.println("technology: " + nmi.getTech() + "\n");

		}

		HamcastUtility.neighborSet(nmi);

		MulticastSocket mSocket = null;

		try {
			mSocket = new MulticastSocket();
		} catch (IOException e) {
			e.printStackTrace();
		}

		// URI uri = new URI("ip://239.0.1.1:1234");
		// mSocket.join(uri);

		URI uri = new URI("scribe://foobar@hallowelt:1234/test");

		System.out.println(uri.getScheme());
		System.out.println(uri.getSchemeSpecificPart());
		System.out.println(uri.getHost());
		System.out.println(uri.getUserInfo());
		System.out.println(uri.getPort());
		System.out.println(uri.getFragment());
		System.out.println(uri.getAuthority());
		System.out.println(uri.getQuery());
		System.out.println(uri.getPath());

		Iterator<GroupSet> elem = HamcastUtility.groupSet(nmi).iterator();
		// Iterator<URI> childIter = MulticastSocket.childrenSet(nmi, uri);
		// Iterator<URI> parentSet = MulticastSocket.parentSet(nmi, uri);

		HamcastUtility.designatedHost(nmi, uri);

		System.out.println("GroupSet für Interface: " + nmi.getDisplayName());

		while (elem.hasNext()) {

			groupSet = elem.next();
			System.out.println("Uri: " + groupSet.getUri().toString()
					+ " Type: " + groupSet.getType());

		}

		mSocket.join(uri);

		while (true) {

			System.out.print(new String(mSocket.receive().getContent()));

		}

	}

}
