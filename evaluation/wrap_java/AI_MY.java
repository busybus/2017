import java.util.ArrayList;
import java.util.List;

public class AI_MY extends Strategy {

	final public static String name = "Clock";
	
	public AI_MY(int C, int N) {
		super(C, N, name);
	}
	
	@Override
	public Response step(int b, List<Integer> B, List<List<Integer>> Q) {
		// Number of passengers to board
		int n = Math.min(Q.get(b).size(), this.C - B.size());
		
		// M is the passenger selection from the queue Q[b]
		List<Integer> M = new ArrayList<>();
		// Take passengers number 0, 1, ..., (n-1)
		for (int i = 0; i < n; i++) M.add(i);
		
		// Always go clockwise
		int s = +1;
		
		return new Response(M, s);
	}	
}

