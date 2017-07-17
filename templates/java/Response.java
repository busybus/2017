import java.util.List;

class Response {
	
	private List<Integer> M;
	private int s;
	
	public Response(List<Integer> M, int s) {
		this.M = M;
		this.s = s;
	}
	
	public List<Integer> getM() {
		return M;
	}
	
	public int getS() {
		return s;
	}
	
}