import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.List;

public class World {
	
	private int busCapacity;      // C
	private int numberOfStations; // N
	private int busPosition;
	private List<Integer> busPassengers;
	private List<List<Integer>> stationsQueues;
	private int i;
	private List<News> NEWS;
	
	class News {
		final int a;
		final int b;
		public News(int a, int b) {
			this.a = a;
			this.b = b;
		}
	}
	
	public World(int busCapacity, int numberOfStations) {
		this.busCapacity = busCapacity;
		this.numberOfStations = numberOfStations;
		this.NEWS = new ArrayList<>();
		this.rewind();
	}
	
	public void rewind() {
		this.busPosition = 0;
		this.busPassengers = new ArrayList<>();
		this.stationsQueues = new ArrayList<>();
		for (int i = 0; i < numberOfStations; i++)
			this.stationsQueues.add(new ArrayList<>());
		this.i = 0;
	}
	
	public News news() {
		while (this.NEWS.size() <= this.i) {
			int N = this.numberOfStations;
			int a = Utils.randInt(0, N - 1);
			int b = (a + Utils.randInt(1, N-1)) % N;
			this.NEWS.add(new News(a, b));
		}
		return this.NEWS.get(i);
	}
	
	public void move(Response response) {
		List<Integer> M = response.getM();
		int s = response.getS();
		
		// http://stackoverflow.com/questions/3018683/what-does-the-assert-keyword-do
		// Assertions have to be enabled with "java -ea Main"
		this.checkConsistency(this.busCapacity, this.numberOfStations, this.busPosition, this.busPassengers, this.stationsQueues, M, s);
		
		for (int i : M) {
			this.busPassengers.add(this.stationsQueues.get(this.busPosition).get(i));
		}
		
		Collections.sort(M, new Comparator<Integer>() {
		    public int compare(Integer n1, Integer n2) {
		        return n2.compareTo(n1);
		    }
		});
		//
		List<Integer> q = this.stationsQueues.get(this.busPosition);
		//
		for (int i : M) q.remove(i);
		
		this.busPosition = (this.busPosition + (this.numberOfStations + s)) % this.numberOfStations;
		
		if (this.busPassengers.size() > 0) {
			this.busPassengers.removeAll(Collections.singleton(this.busPosition));
		}
		
		this.i += 1;
		
		News news = this.news();
		this.stationsQueues.get(news.a).add(news.b);
		
	}
	
	public double getWaitings() {
		ArrayList<Integer> sizes = new ArrayList<>();
		for (List<Integer> q : this.stationsQueues) {
			sizes.add(q.size());
		}
		return Utils.mean(sizes);
	}
	
	public int getBusCapacity() {
		return busCapacity;
	}

	public int getBusPosition() {
		return busPosition;
	}

	public List<Integer> getBusPassengers() {
		return busPassengers;
	}

	public List<List<Integer>> getStationsQueues() {
		return stationsQueues;
	}

	public int geti() {
		return i;
	}

	public void seti(int i) {
		this.i = i;
	}

	public void checkConsistency(int C, int N, int b, List<Integer> B, List<List<Integer>> Q, List<Integer> M, int s) {
		// 0.
        // C is an integer >= 1
        // N is an integer >= 2

        assert(C >= 1);
        assert(N >= 2);

        // 1.
        // b is an integer 0 <= b < N denoting
        //   the current location of the bus.

        assert((0 <= b) && (b < N));

        // 2.
        // B is a list [n1, n2, ..] of
        //   the destinations of the passengers
        //   currently on the bus
        //   (not exceeding the capacity), i.e.
        //   nk is the destination of passenger k.
        //   The order is that of boarding
        //   (provided by this function: see M).
        //   No destination is the current position.

        assert(B.stream().allMatch(n -> ((0 <= n) && (n < N))));
        assert(B.stream().allMatch(n -> (n != b)));
        
        // 3.
        // Q is a list of N lists, where
        //   Q[n] = [t1, t2, ..] is the list of
        //   people currently waiting at station n
        //   with destinations t1, t2, ..
        //   No destination equals the location,
        //   i.e. (t != n) for any t in Q[n].

        assert(Q.size() == N);
        assert(Q.stream().allMatch(q -> q.stream().allMatch(n -> ((0 <= n) && (n < N)))));
        for (int n = 0; n != Q.size(); n++)
			for (int t : Q.get(n))
				assert(t != n);

        // 4.
        // M is a list of indices M = [i1, i2, .., im]
        //   into the list Q[b] indicating that
        //   the people Q[b][i] will board the bus
        //   (in the order defined by M).
        //   Set M = [] if no one boards the bus.
        //   Note the constraints:
        //     len(B) + len(M) <= Capacity C,
        //   and
        //     0 <= i < len(Q[b]) for each i in M.

        assert(M.stream().allMatch(i -> ((0 <= i) && (i < Q.get(b).size()))));
        assert(B.size() + M.size() <= C);

        // 5.
        // s is either +1, -1, or 0, indicating
        //   the direction of travel of the bus
        //   (the next station is (b + s) % N).

        assert((s == -1) || (s == 0) || (s == 1));
	}
}
