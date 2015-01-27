package testing;

import java.net.URI;

import libHamCast.Log;
import libHamCast.MulticastSocket;



public class Benchmark_Test_Receiver {

	public static int unsignedByteToInt(byte b) {
		return (int) b & 0xFF;
	}

	public static int convertArrayToLong(byte[] b, int start, int size) {
		int value = 0;
		for (int i = start; i < start + size; i++) {

			value += unsignedByteToInt(b[i]) << (8 * (i - start));

		}
		return value;
	}

	public static void main(String[] args) throws Exception {

		Log.logLevel = Log.FATAL;

		int random = 10;
		int time = 1000;
		URI uri = new URI("ip://239.0.0.1:1234");

		int ii = 0;
		boolean start = true;

		while (ii < args.length) {

			String arg = args[ii++];

			// Packet Size
			if (arg.equals("-p") || arg.equals("--pass")) {

				random = new Integer(args[ii++]);

			} else if (arg.equals("-h") || arg.equals("--help")) {

				System.out
						.println("-p|--pass set the pass of time triggert receive default=10\n"
								+ "-t|--time [ms] waiting time for display result:1000[ms] \n"
								+ "-u|--uri set the adress to listing default:ip://239.0.1.1:1234\n");
				start = false;
			} else if (arg.equals("-u") || arg.equals("--uri")) {

				uri = new URI(args[ii++]);
			}
		}

		if (start) {

			MulticastSocket mSocket = new MulticastSocket();

			System.out.println("runtime[ms]\tpacket\tloss");

			mSocket.join(uri);
			mSocket.receive();

			long lost = 0;
			byte[] tmp = new byte[4];
			long id = 0;
			long starttime;
			long endtime;
			int i;
			long lastid = 0;

			long sumLost = 0;
			long sumSend = 0;

			Runtime rt = Runtime.getRuntime();

			for (int x = 0; x <= 120; x++) {

				starttime = System.currentTimeMillis();
				endtime = starttime + time;

				i = 0;
				lost = 0;

				for (i = 0; endtime > System.currentTimeMillis(); i++) {

					byte[] packet = mSocket.receive().getContent();

					System.arraycopy(packet, 0, tmp, 0, 4);
					id = convertArrayToLong(tmp, 0, 4);

					lost += id - (lastid + 1);
					lastid = id;

				}

				// long usedMB = (rt.totalMemory() - rt.freeMemory()) / 1024
				// /1024;
				long runtime = System.currentTimeMillis() - starttime;
				System.out.println(runtime + "\t" + i + "\t" + lost + "\t");
				sumSend += i;
				sumLost += lost;

			}

			System.out.println("Summe Send: " + sumSend);
			System.out.println("Summe Lost: " + sumLost);
			float pro = sumSend / 100 * sumLost;
			System.out.println(pro + "% Verlust");

		}

	}
}
