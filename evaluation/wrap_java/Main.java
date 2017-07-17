import java.util.*;
import java.io.*;

public class Main {
	
	public static void main(String[] args) throws FileNotFoundException, IOException {
	
		int C = 0;
		int N = 0;
		int I = 0;

		String filename = "busybusio";
		
		// Stage 1

		{
			Scanner scanner = new Scanner(new File(filename + ".in"));
			C = scanner.nextInt();
			N = scanner.nextInt();
			I = scanner.nextInt();
		}
		
		assert((0 < I) & (I <= 1e9));
		
		World wrd = new World(C, N);
		
		{
			Scanner scanner = new Scanner(new File(filename + ".in"));
			scanner.nextLine();
			while (scanner.hasNextInt()) {
				int a = scanner.nextInt();
				int b = scanner.nextInt();
				assert((0 <= a) & (a < N));
				assert((0 <= b) & (b < N));
				wrd.addNews(a, b);
			}
		}

		
		// Stage 2
		{
			Writer wr = new FileWriter(filename + ".out");
			
			Strategy nav = new AI_MY(C, N);

			wrd.rewind();
			assert(wrd.geti() == 0);
			
			while(wrd.geti() < I)
			{
				int b = wrd.getBusPosition();
				List<Integer> B = wrd.getBusPassengers();
				List<List<Integer>> Q = wrd.getStationsQueues();
				Response r = nav.step(b, B, Q);
				
				int s = r.getS();
				List<Integer> M = r.getM();
				String line = "";
				line += s;
				for (int m : M) line += (" " + m);
				wr.write(line + "\n");
				
				wrd.move(r);
			}
			
			wr.close();
		}
		
	}
}

