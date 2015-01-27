package testing;

import java.io.IOException;
import java.net.URI;
import java.net.URISyntaxException;
import java.util.Iterator;

import libHamCast.HamcastUtility;
import libHamCast.Log;
import libHamCast.MulticastSocket;
import multicastApi.NetworkMInterface;

public class Test_Sender_Performance {

	public int passPacket = 0;
	public int avg = 0;
	public int total = 0;
	public int highest = 0;
	public int lowest = 0;

	public static void main(String[] args) throws IOException {

		Log.logLevel = Log.FATAL;

		URI adress = null;
		try {
			adress = new URI("ip://239.0.0.1:1234");
		} catch (URISyntaxException e2) {
			// TODO Auto-generated catch block
			e2.printStackTrace();
		}
		String packed = "HelloWorld";
		int time = 1000;
		int pass = 20;
		long timeNow = 0;
		int passPacket = 0;
		int avg = 0;
		int total = 0;
		int highest = 0;
		int lowest = 0;

		if (args.length != 0) {

			switch (args.length) {

			case (2):
				time = new Integer(args[1]);
			case (1):
				pass = new Integer(args[0]);

			}
		}

		MulticastSocket mSocket = null;

		mSocket = new MulticastSocket();

		Iterator<NetworkMInterface> netMInter = HamcastUtility
				.getAllInterfaces().iterator();

		while (netMInter.hasNext()) {

			NetworkMInterface nmi = netMInter.next();

			System.out.println("index: " + nmi.getIndex());
			System.out.println("load: " + nmi.getDisplayName());
			System.out.println("adress: " + nmi.getInetAddress());
			System.out.println("technology: " + nmi.getTech() + "\n");

		}

		// Würde die erste Sekunde sehr verfälschen, da beim ersten Senden ein
		// SendStream erstellt wird
		mSocket.send(adress, packed.getBytes());
		try {
			Thread.sleep(10);
		} catch (InterruptedException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}

		Runtime rt = Runtime.getRuntime();
		timeNow = System.currentTimeMillis() + time;

		// System.out.println(Sendbuf.sendBuf.size());

		for (int i = 0; i < pass;) {

			passPacket++;

			mSocket.send(adress, packed.getBytes());

			if (timeNow < System.currentTimeMillis()) {

				if (highest < passPacket || i == 0) {
					highest = passPacket;
				}

				// Damit die ersten Packete nicht gezählt wird!
				// Speicherallokation
				if (passPacket < lowest || i == 1) {
					lowest = passPacket;
				}

				System.out.print("PassPacket: " + passPacket + "/ sec ");
				avg += passPacket;
				passPacket = 0;

				// System.gc();
				long usedMB = (rt.totalMemory() - rt.freeMemory()) / 1024 / 1024;
				System.out.println(" memory usage " + usedMB + " KiloByte ");
				// System.out.println(" Sending: "+IpcSender.sendPointer);

				i++;
				timeNow = System.currentTimeMillis() + time;
				// System.out.println(Sendbuf.sendBuf.size());

			}
		}

		total = avg;
		avg = avg / pass;

		System.out.println("\ntotal: " + total);
		System.out.println("avg: " + avg);
		System.out.println("lowest: " + lowest);
		System.out.println("highest: " + highest);

		// System.out.println(Sendbuf.sendBuf.size());
		mSocket.destroy();

	}

}
