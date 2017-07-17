
// Template by verybusybus.wordpress.com

// Author: Björn Martinsson and Anton Älgmyr
// License: MIT License

#include <numeric>
#include <algorithm>
#include <random>

// Create a random engine to simulate
default_random_engine RNGeesus(random_device{}());
unsigned randinte(unsigned A, unsigned B) {
	uniform_int_distribution<unsigned> D(A, B);
	return D(RNGeesus);
}

struct AI_MY : public Strategy {
	/*
	 * A somewhat simple stratergy:
	 * BASIC IDEA:
	 *   1. Go back and forth with turn_const number of steps
	 *   2. Pick up passengers in order of how close they
	 *      are to their destination
	 *   3. Only pick up passengers with a destination <= pickup_const away
	 *   (Notice that distance is dependent on the direction of the bus)
	 *
	 * The constants turn_const and pickup_const is found by trial and error
	 * using simulations. Basically try every reasonable combination of
	 * contsants. Notice that we use the same seed for every simulation
	 * to make them comparable to eachother.
	 *
	 * EXTRA OPTIMIZATION:
	 *   Because the original algorithm was very simple and could clearly
	 *   be optimized, we added 4 constants to make bus turn earlier if
	 *   ("many people waiting want the bus to go the other way" &&
	 *    "few passenger on the bus oposes the bus turning").
	 *   The definition of "many" and "few" and "want the bus" are all
	 *   given by the constants. The optimal definition is found in the
	 *   same way as before, using simulations.
	 *
	 */
	
	unsigned C;
	unsigned N;
	
	// The basic constants
	unsigned pickup_const;
	unsigned turn_const;
	
	// The advanced constants, making the bus turn earlier
	unsigned C1,C2,C3,C4;
	
	// How mant timesteps left until turning
	unsigned until_turn;
	// What direction is the bus going in
	int dir;
	
	// If this object is used for simulation
	bool is_drill;
	
	// Copy a previously made object
	AI_MY(AI_MY* original, bool _is_drill) : Strategy("MY SECRET EXPERIMENT") {
		C = original->C;
		N = original->N;
		
		until_turn = original->until_turn;
		
		pickup_const = original->pickup_const;
		turn_const = original->turn_const;
		dir = original->dir;
		
		C1 = original->C1;
		C2 = original->C2;
		C3 = original->C3;
		C4 = original->C4;
		
		// If this object is used for simulation
		is_drill = _is_drill;
	}
	
	
	AI_MY(unsigned _C, unsigned _N) : Strategy("MY UNNAMED STRATEGY")
	{
		C = _C;
		N = _N;
		
		// Set some reasonable parameters
		dir = 1;
		pickup_const = (3*N)/4;
		turn_const = 3*N+1;
		until_turn = turn_const;
		C1 = 4;
		C2 = 1;
		C3 = 3;
		C4 = 14;
		is_drill = false;
		
		// Prepare for simulation
		int seed = RNGeesus();  // Find the seed to be used for all simulations
		unsigned iterr = 100000; // Numbers of iterations
		
		// Simulate the reasonalbe choice of parameters
		AI_MY tester(this,true);
		RNGeesus.seed(seed);
		unsigned best_score = tester.simulate(iterr, iterr*iterr+1);
		
		// Try out with different parameters
		for (unsigned _pickup_const = (N+1)/2; _pickup_const <= N; ++_pickup_const) {
			for (unsigned _turn_const = 2*N+1; _turn_const < 10*N; _turn_const+=5) {
				RNGeesus.seed(seed);
				tester = AI_MY(this,true);
				tester.pickup_const = _pickup_const;
				tester.turn_const   = _turn_const;
				tester.until_turn   = _turn_const;
				unsigned score = tester.simulate(iterr, best_score);
				if (score<best_score) {
					// Found a better set of parameters
					// cout << "Improved score " << score << endl;
					best_score = score;
					turn_const = _turn_const;
					until_turn = turn_const;
					pickup_const = _pickup_const;
				}
			}
		}
		
		for (unsigned _C1 = 0; _C1 <= N/2; ++_C1) {
			for (unsigned _C2 = 0; _C2 < 3; ++_C2) {
				RNGeesus.seed(seed);
				tester = AI_MY(this,true);
				tester.C1 = _C1;
				tester.C2 = _C2;
				unsigned score = tester.simulate(iterr, best_score);
				if (score<best_score) {
					// Found a better set of parameters
					// cout << "Improved score " << score << endl;
					best_score = score;
					C1 = _C1;
					C2 = _C2;
				}
			}
		}
		for (unsigned _C3 = 0; _C3 < N; ++_C3) {
			for (unsigned _C4 = 0; _C4 <= N; ++_C4) {
				RNGeesus.seed(seed);
				tester = AI_MY(this,true);
				tester.C3 = _C3;
				tester.C4 = _C4;
				unsigned score = tester.simulate(iterr, best_score);
				if (score<best_score) {
					// Found a better set of parameters
					// cout << "Improved score " << score << endl;
					best_score = score;
					C3 = _C3;
					C4 = _C4;
				}
			}
		}
		
		for (unsigned _pickup_const = (N+1)/2; _pickup_const <= N; ++_pickup_const) {
			for (unsigned _turn_const = 2*N+1; _turn_const < 10*N; _turn_const+=5) {
				RNGeesus.seed(seed);
				tester = AI_MY(this,true);
				tester.pickup_const = _pickup_const;
				tester.turn_const   = _turn_const;
				tester.until_turn   = _turn_const;
				unsigned score = tester.simulate(iterr, best_score);
				if (score<best_score) {
					// Found a better set of parameters
					// cout << "Improved score " << score << endl;
					best_score = score;
					turn_const = _turn_const;
					until_turn = turn_const;
					pickup_const = _pickup_const;
				}
			}
		}
		
		// All parameters optimized and ready
	}
	
	/* Do the same simulation as in the real test. */
	unsigned simulate(unsigned iter, unsigned best_score, unsigned b, Targets B, vector<Targets> Q)
	{
		unsigned waiting = 0;
		for (Targets q : Q) waiting += q.size();
		
		
		unsigned score = 0;
		
		for (int i = 0; i < iter; ++i) {
			// Take a step
			Response r = step(b,B,Q);
			Targets &new_pass = r.M;
			int dire = r.s;
			
			// Fill up the bus
			waiting -= new_pass.size();
			
			// Add passengers
			sort(new_pass.rbegin(), new_pass.rend());
			for (auto i : new_pass) {
				B.push_back(Q[b][i]);
				Q[b].erase(Q[b].begin()+i);
			}
			
			// Move the bus
			b = (b+N+dire)%N;
			
			// Terminate passengers
			Targets Bnew;
			for (auto dest : B) {
				if (dest != b) {
					Bnew.push_back(dest);
				}
			}
			B = Bnew;
			
			// Add one more passenger
			unsigned rand_station = randinte(0, N-1);
			unsigned rand_pass_dest = (rand_station + randinte(1, N-1)) % N;
			Q[rand_station].push_back(rand_pass_dest);
			++waiting;
			
			score += waiting;
			if (score>best_score) break;
		}
		
		return score;
	}
	
	/* Wrapper to simulate without any initial condition */
	unsigned simulate(unsigned iter, unsigned best_score)
	{
		unsigned b = 0; // Pos
		Targets B; // In bus
		vector<Targets> Q = vector<Targets>(N); // Queues
		
		return simulate(iter, best_score, b, B, Q);
	}
	
	
	
	Response step(unsigned b, const Targets& B, const vector<Targets>& Q)
	{
		// Count down to start turning
		--until_turn;
		if (until_turn <= 0) {
			// turn around
			until_turn = turn_const;
			dir *= -1;
		}
		
		
		const Targets &current = Q[b];
		unsigned num = current.size();
		unsigned curr_pass = B.size();
		
		// Distance to destination, (time until the bus reaches dest)
		auto dist = [this,b](unsigned dest,int dire) -> unsigned {
			return (dire*(dest-b) + 10*this->N)%this->N;
		};
		
		// Number of passengers not wanting the bus to turn around
		unsigned opposing = 0;
		for (auto dest : B) {
			opposing += dist(dest,dir)<=C1;;
		}
		
		// Number of people waiting wanting the bus to turn around
		// minus number of waiting not wanting the bus to turn around.
		int should_turn = 0;
		for (int i = 0; i < N; ++i){
			for(auto q : Q[i]){
				should_turn += (((dir*(q-i)+N)%N)<C4)?-1:1;
			}
		}
		
		// Check if we can turn early
		if (should_turn >= (int)C3 && opposing <= C2) {
			dir *= -1;
			until_turn = turn_const;
		}
		
		
		// Order people waiting in the depending on how close they are to
		// their destination
		vector<int> indices(num);
		iota(indices.begin(), indices.end(), 0);
		sort(indices.begin(), indices.end(), [&dist,&current,this](int i, int j){
			return dist(current[i],this->dir)<dist(current[j],this->dir);
		});
		
		Targets new_pass;
		
		// Try to pick up passengers
		for (int i = 0; i < indices.size(); ++i) {
			if (curr_pass >= C)
				// Bus is full
				break;
			unsigned pass_id = indices[i];
			
			// Be picky on who to pick up
			if (dist(current[pass_id],dir) <= min(pickup_const,until_turn)) {
				new_pass.push_back(pass_id);
				++curr_pass;
			} else {
				// The waiting people are ordered, so if
				// we dont pick up one, then we are
				// not going to pick up any more
				break;
			}
		}
		
		return Response(new_pass, dir);
	}
};
