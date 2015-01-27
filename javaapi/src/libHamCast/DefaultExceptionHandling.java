package libHamCast;

public class DefaultExceptionHandling implements Runnable {

	@Override
	public void run() {
		
		
		throw new RuntimeException(
				"Couldn't reach middleware - is it Running?");
		
		} 
		

}
