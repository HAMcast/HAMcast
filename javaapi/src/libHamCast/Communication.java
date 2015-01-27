package libHamCast;

import java.io.BufferedReader;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;
/*
 * Handles the Communication to  the middleware directly.
 * parsing etc.
 *  */

class Communication {

	protected MiddlewareStream middlewareStream;

	protected IpcSyncBuffer ipcSyncBuffer;
	protected AsyncReceiveBuf ipcAsyncReceiveBuf;

	public Communication(String adress) throws IOException  {

		ipcSyncBuffer = new IpcSyncBuffer();
		ipcAsyncReceiveBuf = new AsyncReceiveBuf();

		String substring = "";
		int first = 0;
		int port = 0;
		
		BufferedReader br;
		try {
			br = new BufferedReader(new FileReader(
					"/tmp/hamcast/meeting_point/middleware/middleware.config_file"));
		} catch (FileNotFoundException e) {
			throw new IOException("couldn't find configfile. Is middleware running?");
			
		}

		substring = br.readLine();
		first = substring.lastIndexOf(' ') + 1;
		substring = substring.substring(first);
		port = Integer.parseInt(substring);

		substring = br.readLine();
		first = substring.lastIndexOf(' ') + 1;
		substring = substring.substring(first);
		// pid = Integer.parseInt(substring);

		middlewareStream = new MiddlewareStream(adress, port,
				Parameter.m_magic_number, Parameter.m_major_version,
				Parameter.m_minor_version);

	}

}
