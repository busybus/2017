import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.List;

public class World {
	
	private int busCapacity;
	private int numberOfStations;
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
		
		for (int i : response.getM()) {
			this.busPassengers.add(this.stationsQueues.get(this.busPosition).get(i));
		}
		
		List<Integer> M = response.getM();
		Collections.sort(M, new Comparator<Integer>() {
		    public int compare(Integer n1, Integer n2) {
		        return n2.compareTo(n1);
		    }
		});
		List<Integer> S = this.stationsQueues.get(this.busPosition);
		for (int i : M) {
			S.remove(i);
		}
		
		this.busPosition = (this.busPosition + (this.numberOfStations + response.getS())) % this.numberOfStations;
		
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

	public void setBusPosition(int busPosition) {
		this.busPosition = busPosition;
	}

	public List<Integer> getBusPassengers() {
		return busPassengers;
	}

	public void setBusPassengers(List<Integer> busPassengers) {
		this.busPassengers = busPassengers;
	}

	public List<List<Integer>> getStationsQueues() {
		return stationsQueues;
	}

	public void setStationsQueues(List<List<Integer>> stationsQueues) {
		this.stationsQueues = stationsQueues;
	}

	public int getI() {
		return i;
	}

	public void setI(int i) {
		this.i = i;
	}
	
}
