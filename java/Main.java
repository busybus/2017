import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

public class Main {
	
	final static int C = 3;
	final static int N = 6;
	
	static class Score implements Comparable<Score> {
		int contestant;
		double score;
		public Score(int contestant, double score) {
			this.contestant = contestant;
			this.score = score;
		}
		
		@Override
		public int compareTo(Score o) {
			double d = this.score - o.score;
			// Return the sign of the difference d
			return ((d > 0) ? 1 : (d < 0) ? -1 : 0);
		}
	}
	
	public static void main(String[] args) {
		
		List<Strategy> strategies = new ArrayList<>();
		strategies.add(new AI_CLOCK(C, N));
		strategies.add(new AI_KLOCK(C, N));
		strategies.add(new AI_GREEDY(C, N));
		
		
		// The following block computes for each contestant n:
		// 1. The rank in ranks.get(n)
		List<Integer> ranks = new ArrayList<>();
		// 2. The score history in scores.get(n)
		List<List<Double>> scores = new ArrayList<>();
		
		System.out.println("Starting competition");
		{
			
			// Initialize ranks and scores
			while (ranks.size() < strategies.size()) ranks.add(0);
			while (scores.size() < strategies.size()) scores.add(new ArrayList<Double>());
			
				
			// Next rank to be assigned is
			// the number of currently unassigned ranks
			int rank;
			// Continue while that number is non-zero
			while ((rank = Collections.frequency(ranks, 0)) > 0) {
				System.out.println("Number of competitors: " + rank);
			
				World world = new World(C, N);
			
				// Strategy scores for this round
				// (nonnegative; max score loses)
				List<Score> K = new ArrayList<>();
			
				for (int n = 0; n < strategies.size(); ++n) {
					// Only contestants with unassigned rank participate
					if (ranks.get(n) > 0) continue;
					
					Strategy strategy = strategies.get(n);
				
					System.out.println(" - Profiling " + strategy.name);
				
					Profiler report = new Profiler(world, strategy);
					double score = report.w;

					K.add(new Score(n, score));
					System.out.println("   *Score for this round: " + score);
				}
				assert(!K.isEmpty());
			
				Score maxScore = Collections.max(K);
			
				for (Score score : K) {
					int n = score.contestant;
					
					// Rank is assigned if this was
					// the contestant's last round
					if (score == maxScore) ranks.set(n, rank);
					
					// Add the newest score to the history of contestant n
					List<Double> scores_of_n = scores.get(n);
					scores_of_n.add(score.score);
					// REPLACE it by the average so far (to have a more "stable" number)
					scores_of_n.set(scores_of_n.size()-1, Utils.mean(scores_of_n));
				}
			}
			
		}
		
		System.out.println("Final ranking:");
		for (int rank = 1; rank <= ranks.size(); ++rank) {
			System.out.println("Rank " + rank);
			// Show the strategies whose rank is rank
			for (int n = 0; n != ranks.size(); ++n) {
				if (ranks.get(n) != rank) continue;
				System.out.println("   " + strategies.get(n).getName());
			}
		}
		
	}
	
}
