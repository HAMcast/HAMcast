package testing;

import java.io.BufferedWriter;
import java.io.FileWriter;
import java.io.IOException;
import java.net.URI;
import java.util.ArrayList;
import java.util.List;

import libHamCast.HamcastUtility;
import libHamCast.Log;
import libHamCast.MulticastSocket;
import multicastApi.NetworkMInterface;

public class PureReceiver {
	public static void main(String[] args) throws Exception{
		MulticastSocket mSocket=new MulticastSocket();
		Log.log(Log.TRACE, "Logging.txt");

		/*		int localhostid=23;
		List<NetworkMInterface> bla=HamcastUtility.getAllInterfaces();
		for(NetworkMInterface nmi: bla){
			if(nmi.getDisplayName().startsWith("lo"))localhostid=nmi.getIndex();
		}

		List<Integer> ids=new ArrayList<Integer>();
		ids.add(localhostid);
		mSocket.setInterfaces(ids);
		System.out.println(mSocket.getInterfaces());
		 */
		//	int i=1234;

		for(int i=1234; i<1244;i++){
			mSocket.join("ip://239.0.0.1:"+String.valueOf(i));
			System.out.println("joined ip://239.0.0.1:"+String.valueOf(i));
		}
		while(true){
			String str=new String(mSocket.receive().getContent());
			System.out.println(str);
			FileWriter fstream = new FileWriter("out.txt");
			BufferedWriter out = new BufferedWriter(fstream);
			out.write(new String(str+"\n"));
			out.close();
		}
	}
}
