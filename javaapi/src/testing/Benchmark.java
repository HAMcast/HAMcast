package testing;

import java.net.URI;

import libHamCast.Log;
import libHamCast.MulticastSocket;

public class Benchmark {

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
		long stopp = 0;
		URI uri = new URI("ip://239.0.0.1:1234");
		boolean start = true;
		byte[] packet = new byte[1200];
		boolean mode = true;
		boolean endless = true;

		while (ii < args.length) {

			String arg = args[ii++];

			// Packet Size
			if (arg.equals("-b") || arg.equals("--byte")) {

				packetsize = new Integer(args[ii++]);

			} else if (arg.equals("-h") || arg.equals("--help")) {

				System.out
						.println("-b|--byte packetsize[Byte] default=1200\n"
								+ "-i|--id first packet start with id default:0 \n"
								+ "-d|--delay set the delay[ms] time between two packets default:0\n"
								+ "-c|--count  count of sendpacket default: endless(0)\n"
								+ "-u|--uri set the listener adress default:ip://239.0.0.1:1234\n"
								+ "-l|--log set the apilog level 0:fatal 1:error 2:warning 3:info 4:debug 5:trace default:fatal \n"
								+ "-t|--time [ms] after Stop endless:0 default:0 \n"
								+ "-r|--receiver Receiver mode  default: receiver \n"
								+ "-s|--sender Sendermode  default: receiver \n");
				start = false;

			} else if (arg.equals("-i") || arg.equals("--id")) {

				id = new Integer(args[ii++]);

			}

			else if (arg.equals("-d") || arg.equals("--delay")) {

				delay = new Integer(args[ii++]);

			} else if (arg.equals("-u") || arg.equals("--uri")) {

				uri = new URI(args[ii++]);

			} else if (arg.equals("-l") || arg.equals("--log")) {

				Log.printFile = true;
				Log.logLevel = new Integer(args[ii++]);

			} else if (arg.equals("-t") || arg.equals("--time")) {

				stopp = new Integer(args[ii++]);
				if (stopp == 0) {
					endless = true;
				} else {
					endless = false;
				}

			}

			else if (arg.equals("-r") || arg.equals("--receiver")) {

				mode = true;

			}

			else if (arg.equals("-s") || arg.equals("--sender")) {

				mode = false;

			}

		}

		if (start) {

			if (!mode) {

				// Sender

				MulticastSocket mSocket = new MulticastSocket();

				System.out.println("#runtime[ms]\tpacket\tloss\tmemory");

				long lost = 0;
				byte[] tmp = new byte[4];

				long cycleTime = 0;
				int i;

				long totaltime = System.currentTimeMillis() + stopp;

				while ((System.currentTimeMillis() < totaltime || endless)) {

					long starttime = System.currentTimeMillis();
					long endtime = starttime + 1000;

					i = 0;

					for (i = 0; endtime > System.currentTimeMillis(); i++) {
						packet = new byte[packetsize];
						tmp = uint32ToByteArray(id++);
						System.arraycopy(tmp, 0, packet, 0, 4);
						mSocket.send(uri, packet);
						Thread.sleep(delay);

					}

					cycleTime += System.currentTimeMillis() - starttime;
					System.out.println(cycleTime + "\t" + i + "\t" + lost
							+ "\t");

				}

			} else {

				// Receiver

				MulticastSocket mSocket = new MulticastSocket();

				System.out.println("runtime[ms]\tpacket\tloss");

				mSocket.join(uri);
				mSocket.receive();

				long lost = 0;
				byte[] tmp = new byte[4];
				int i;
				long lastid = 0;

				long sumLost = 0;
				long sumSend = 0;
				long cycleTime = 0;

				long totaltime = System.currentTimeMillis() + stopp;

				while ((System.currentTimeMillis() < totaltime || endless)) {

					long starttime = System.currentTimeMillis();
					long endtime = starttime + 1000;

					i = 0;
					lost = 0;

					for (i = 0; endtime > System.currentTimeMillis(); i++) {

						byte[] rPacket = mSocket.receive().getContent();

						System.arraycopy(rPacket, 0, tmp, 0, 4);
						id = convertArrayToLong(tmp, 0, 4);
						rPacket = null;

						lost += id - (lastid + 1);
						lastid = id;

					}

					cycleTime += System.currentTimeMillis() - starttime;
					String str = cycleTime + "\t" + i + "\t" + lost + "\t";
					System.out.println(str);
					Log.log(Log.TRACE, str);
					sumSend += i;
					sumLost += lost;

				}

			}
		}
	}

}