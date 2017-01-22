import java.util.ArrayList;
import java.util.List;

public class AI_GREEDY extends Strategy {

	final public static String name = "Greedy";
	
	public AI_GREEDY(int C, int N) {
		super(C, N, name);
	}
	
	@Override
	public Response step(int b, List<Integer> B, List<List<Integer>> Q) {
		int n = Math.min(Q.get(b).size(), this.C - B.size());
		List<Integer> M = new ArrayList<>();
		for (int i = 0; i < n; i++)
			M.add(i);
		
		if (B.isEmpty() && M.isEmpty()) return new Response(M, 1);
		
		int t;
		if (!B.isEmpty())
			t = B.get(0);
		else
			t = Q.get(b).get(M.get(0));
		
		t = N - 2 * ((t+N-b) % N);
		int s = (t > 0)? 1:-1;
		
		return new Response(M, s);
	}

}
