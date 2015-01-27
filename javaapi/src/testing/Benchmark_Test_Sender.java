package testing;

import java.net.URI;

import libHamCast.Log;
import libHamCast.MulticastSocket;

public class Benchmark_Test_Sender {

	public static byte[] uint32ToByteArray(long value) {
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

	public static void main(String[] args) throws Exception {

		Log.logLevel = Log.FATAL;

		int ii = 0;
		int packetsize = 0;
		long id = 0;
		int delay = 0;
		int count = 0;
		int time = 1000;
		URI uri = new URI("ip://239.0.0.1:1234");
		boolean start = true;
		byte[] packet = new byte[1200];

		while (ii < args.length) {

			String arg = args[ii++];

			// Packet Size
			if (arg.equals("-s") || arg.equals("--size")) {

				packetsize = new Integer(args[ii++]);
				packet = new byte[packetsize];

			} else if (arg.equals("-h") || arg.equals("--help")) {

				System.out
						.println("-s|--size packetsize[Byte] default=1200\n"
								+ "-i|--id first packet start with id default:0 \n"
								+ "-d|--delay set the delay[ms] time between two packets default:0\n-c|--count  count of sendpacket default: endless(0)"
								+ "-u|--uri set the listener adress default:ip://239.0.1.1:1234\n"
								+ "-l|--log set the apilog level 0:fatal 1:error 2:warning 3:info 4:debug 5:trace default:fatal \n"
								+ "-b|--buffer set the size of the API Sendbuffer\n "
								+ "-t|--time [ms] waiting time for display result:1000[ms] \n");
				start = false;

			} else if (arg.equals("-i") || arg.equals("--id")) {

				id = new Integer(args[ii++]);

			}

			else if (arg.equals("-d") || arg.equals("--delay")) {

				delay = new Integer(args[ii++]);

			} else if (arg.equals("-c") || arg.equals("--count")) {

				count = new Integer(args[ii++]);

			} else if (arg.equals("-u") || arg.equals("--uri")) {

				uri = new URI(args[ii++]);

			} else if (arg.equals("-l") || arg.equals("--log")) {

				Log.logLevel = new Integer(args[ii++]);

			} else if (arg.equals("-t") || arg.equals("--time")) {

				time = new Integer(args[ii++]);

			} else if (arg.equals("-b") || arg.equals("--buffer")) {

				// buffersize = new Integer(args[ii++]);

			}

		}

		if (start) {

			MulticastSocket mSocket = new MulticastSocket();

			System.out.println("#runtime[ms]\tpacket\tloss\tmemory");

			long lost = 0;
			byte[] tmp = new byte[4];

			long starttime;
			long endtime;
			int i;
			Runtime rt = Runtime.getRuntime();

			for (;;) {

				starttime = System.currentTimeMillis();
				endtime = starttime + time;
				i = 0;

				for (i = 0; endtime > System.currentTimeMillis(); i++) {

					tmp = uint32ToByteArray(id++);
					System.arraycopy(tmp, 0, packet, 0, 4);
					mSocket.send(uri, packet.clone());

				}

				long runtime = System.currentTimeMillis() - starttime;
				System.out.println(runtime + "\t" + i + "\t" + lost + "\t");

			}

		}

	}

}
