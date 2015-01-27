package libHamCast;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.io.PrintStream;
import java.io.Writer;
import java.text.SimpleDateFormat;
import java.util.Date;
/**
 * Logger for specialized output.<br>
 * Logging levels:
 * <ul>
 * <li> TRACE
 * <li> DEBUG
 * <li> INFO
 * <li> WARN
 * <li> ERROR
 * <li> FATAL
 * </ul>
 * */
public class Log {

	// LOG Level
	public static final int TRACE = 5;
	public static final int DEBUG = 4;
	public static final int INFO = 3;
	public static final int WARN = 2;
	public static final int ERROR = 1;
	public static final int FATAL = 0;

	public static int logLevel = FATAL;

	static final PrintStream prt = System.out;
	static SimpleDateFormat formatter = null;

	public static final boolean ACTIVE = true;

	public static boolean printConsole = false;
	public static boolean printFile = false;

	static File file = null;
	static Writer writer = null;

	public static synchronized void log(int level, String str) {

		if (level <= logLevel) {
			if (printConsole) {
				formatter = new SimpleDateFormat("yyyy.MM.dd '::' HH:mm:ss :: ");
				prt.println(formatter.format(new Date()) + str);
		
			}

			if (printFile) {
				try {
					// legt eine neue Datei an
					if (file == null) {
						file = new File("Logging.txt");

						writer = new FileWriter(file);

					}

					formatter = new SimpleDateFormat(
							"yyyy.MM.dd '::' HH:mm:ss :: ");
					writer.write(formatter.format(new Date()) + str
							+ System.getProperty("line.separator"));
					writer.flush();
			
				} catch (IOException e) {

					e.printStackTrace();
				}

			}

		}

	}

}
