package examples;

import java.net.URI;
import libHamCast.MulticastSocket;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;


public class HC_Chat{
	public static void main(String[] args){

		try{
			BufferedReader br = new BufferedReader(new InputStreamReader(System.in));
			System.out.println("Just type, press enter, and enjoy :) ");
			MulticastSocket mSocket=new MulticastSocket();
			String uri="ip://239.0.0.1:1234";
			mSocket.join(uri);
			Receiver receiver=new Receiver(mSocket, "receiver");
		//	receiver.run();
			
			while (true) {

				String message=br.readLine();
			
				mSocket.send(uri, message.getBytes());
				
				}
		}
		catch(Exception e){
			e.printStackTrace();
		}

	}
	
	
	
}
class Receiver implements Runnable {

	Thread runner;
	MulticastSocket socket;

	public Receiver() {
	}
	public Receiver(MulticastSocket socket, String threadName) {
		runner = new Thread(this, threadName); // (1) Create a new thread.
		this.socket=socket;
		//System.out.println(runner.getName());
		runner.start(); // (2) Start the thread.
	}
	public void run() {
		//Display info about this particular thread
		//System.out.println(Thread.currentThread());
		while(true){
		try {
			byte[] msg=this.socket.receive().getContent();
			String real_msg=new String(msg);
			System.out.println("new message: "+real_msg);
			
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		}
	}
}
