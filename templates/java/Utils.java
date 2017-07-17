import java.util.List;
import java.util.concurrent.ThreadLocalRandom;

public class Utils {
	
	public static double mean(List<? extends Number> list) {
		assert (!list.isEmpty()) : "Empty List";
		
		double sum = 0;
		for (Number e : list) 
			sum += e.doubleValue();
		return sum / list.size();

	}
	
	// Return a random integer from the interval [a, b]
	public static int randInt(int a, int b) {
		return ThreadLocalRandom.current().nextInt(a, b + 1);
	}
	
}
