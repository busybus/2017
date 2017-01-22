import java.util.ArrayList;
import java.util.List;

public class AI_CLOCK extends Strategy {

	final public static String name = "Clock";
	
	public AI_CLOCK(int C, int N) {
		super(C, N, name);
	}
	
	@Override
	public Response step(int b, List<Integer> B, List<List<Integer>> Q) {
		int n = Math.min(Q.get(b).size(), this.C - B.size());
		List<Integer> M = new ArrayList<>();
		for (int i = 0; i < n; i++)
			M.add(i);
		return new Response(M, 1);
	}	

}
