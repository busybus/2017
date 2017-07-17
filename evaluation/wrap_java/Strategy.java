import java.util.List;

public abstract class Strategy {
	
	final public int C;
	final public int N;
	final String name;
	
	public Strategy(int C, int N, String name) {
		this.C = C;
		this.N = N;
		this.name = name;
	}
	
	public abstract Response step(int b, List<Integer> B, List<List<Integer>> Q);
	
	public String getName() {
		return this.name;
	}
	
}
