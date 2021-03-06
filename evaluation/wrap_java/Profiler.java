import java.util.ArrayList;
import java.util.List;

public class Profiler {
	
	/**
	 * Number of iterations (time steps)
	 * This will be I ~ 1e6
	 */
	final static int I = 100000;
	
	// W[i] = average number of people waiting at time i
	final public List<Double> W;
	
	// Average over time
	final double w;
	
	public Profiler(World world, Strategy strategy) {
		// Runs the system with a particular strategy
		
		// W[i] = average number of people waiting at time i
		this.W = new ArrayList<>();
		
		world.rewind();
		assert(world.geti() == 0);
		
		// Main loop
		while(world.geti() < Profiler.I)
		{
			int b = world.getBusPosition();
			List<Integer> B = world.getBusPassengers();
			List<List<Integer>> Q = world.getStationsQueues();
			world.move(strategy.step(b, B, Q));
			W.add(world.getWaitings());
		}
		
		assert(!W.isEmpty());
		
		this.w = Utils.mean(this.W);
	}
	
}
