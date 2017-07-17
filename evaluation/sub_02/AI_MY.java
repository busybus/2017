// Author: Rui Viana
// License: CC BY 4.0

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

public class AI_MY extends Strategy {
	abstract static class Scoreable implements Comparable<Scoreable> {
		double score;
		public int compareTo(Scoreable s) {return Double.compare(this.score, s.score);}
	}

	static class Path extends Scoreable {
		List<Integer> stations,M = new ArrayList<>();
		public Path(List<Integer> stations, double scoreAdder) {
			this.score = scoreAdder;
			this.stations = stations;
		}
	}

	static class CarryOption extends Scoreable {
		int from_idx, to_idx;
		public CarryOption(int f, int t, int score) {
			this.score = score;
			this.from_idx = f;
			this.to_idx = t; 
		}
	}

	static class Passenger extends Scoreable {
		int from_st, to_st, qidx;
		List<CarryOption> options;
		CarryOption chosenOption;
		public Passenger(int from_st, int to_st, int qidx, List<CarryOption> options) {
			this.from_st = from_st;
			this.to_st = to_st;
			this.qidx = qidx;
			this.options = options;
			this.score = options.get(0).score;
		}
	}

	final public static String name = "Rui's AI";
	int path_len,lastDir=1;
	double[] drop_ws;
	int[] occup, drops;

	public AI_MY(int C, int N) {
		super(C, N, name);
		this.path_len = (int) (N * 1.25);
		this.occup = new int[this.path_len];
		this.drops = new int[this.path_len];
		this.drop_ws = new double[this.path_len];
		double disc = 1.02;
		for(int idx = 0; idx < drop_ws.length; idx++) 
			this.drop_ws[idx] = 1.0 - (1.0-Math.pow(disc,-idx))/(1.0-Math.pow(disc,-this.path_len+1));
	}
	
	public static boolean can_carry(int from, int to, int[] occup, int C) {
		if(from >= to) return false;
		for (int idx = from; idx < to; idx++)
			if (occup[idx] == C) 
				return false;
		for (int idx = from; idx < to; idx++) 
			occup[idx]++;
		return true;
	}

	public void evaluate_path(List<Integer> B, List<List<Integer>> Q, Path path) {
		for(int i = 0; i < this.path_len; i++) this.occup[i] = 0;
		for(int i = 0; i < this.path_len; i++) this.drops[i] = 0;

		// first calculate for how long each passenger will be on the bus
		for (int psg : B) {
			for (int pidx = 0; pidx < path.stations.size(); pidx++) {
				if (path.stations.get(pidx) == psg) {
					drops[pidx]++;
					break;
				}
				occup[pidx] += 1;
			}
		}

		// make a list of the passengers and when we can pick them up		
		List<Passenger> passengers = new ArrayList<>();
		for (int from_st = 0; from_st < Q.size(); from_st++) {
			List<Integer> stQ = Q.get(from_st);
			
			List<Integer> visits = new ArrayList<>();			
			for (int pidx = 0; pidx < path.stations.size(); pidx++)
				if (path.stations.get(pidx) == from_st) 
					visits.add(pidx);
				
			for (int qidx = 0; qidx < stQ.size(); qidx++) {
				int to_st = stQ.get(qidx);
				List<CarryOption> options = new ArrayList<>();
				for(int pidx1 : visits) 
					for (int pidx2 = pidx1 + 1; pidx2 < path.stations.size(); pidx2++)
						if (path.stations.get(pidx2) == to_st)
							options.add(new CarryOption(pidx1, pidx2, (pidx2-pidx1)*path_len+pidx1));

				if (options.isEmpty()) continue;
				Collections.sort(options);
				passengers.add(new Passenger(from_st, to_st, qidx, options));
			}
		}
		Collections.sort(passengers);

		List<Passenger> picked = new ArrayList<>();
		for(Passenger psg : passengers) {
			for (CarryOption co : psg.options) {
				if(!can_carry(co.from_idx, co.to_idx, occup, this.C)) continue; 
				psg.chosenOption = co;
				picked.add(psg);
				drops[psg.chosenOption.to_idx]++;
				break; // break because each passenger can only be carried once
			}
		}
		
		for(Passenger psg : picked) // see if passengers can be picked up earlier
			for(CarryOption co : psg.options)
				if(can_carry(co.from_idx, psg.chosenOption.from_idx, occup, this.C)) 
					psg.chosenOption = co;
		
		for(Passenger psg : picked) 
			if(psg.chosenOption.from_idx==0)
				path.M.add(psg.qidx);
		
		for (int idx = 0; idx < drops.length; idx++) 
			path.score += drops[idx] * this.drop_ws[idx];
	}

	public Response step(int b, List<Integer> B, List<List<Integer>> Q) {
		List<Path> options = new ArrayList<>();
		int[] dirs = {this.lastDir,this.lastDir*(-1)};
		for(int direction : dirs) {			
			List<Integer> path = new ArrayList<>();
			for (int disp = 0; disp < path_len; disp++) 
				path.add((b + disp * direction + this.N*10) % this.N);
			options.add(new Path(path, direction==this.lastDir? 0.2 : 0.0));
			
			int max_back = (int)(0.35 * this.N);
			if(direction!=this.lastDir) max_back = 1;
			for(int back = 1; back <= max_back; back++) {
				List<Integer> path2 = new ArrayList<>();
				for (int disp = 0; disp <= back; disp++) 
					path2.add((b + disp * direction + this.N*10) % this.N);
				for (int disp = 1; disp < path_len - back; disp++) 
					path2.add((b + back * direction - disp * direction + this.N*10) % this.N);
				options.add(new Path(path2, direction==this.lastDir? -0.08 : 0.0));
			}
		}

		for (Path p : options) this.evaluate_path(B, Q, p);
		Path winner = Collections.max(options);
		this.lastDir = (winner.stations.get(1) - winner.stations.get(0) + 1 + this.N*10) % this.N - 1;
		return new Response(winner.M, this.lastDir);
	}
}
