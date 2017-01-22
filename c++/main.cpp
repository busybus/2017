
// Template by verybusybus.wordpress.com

// Author:
// TODO: SAVE THIS FILE TO YOUR MACHINE
//       WRITE YOUR NAME(S) HERE 


#include <iostream>
using namespace std;

// Vector of passenger destinations
#include <vector>
typedef vector<unsigned> Targets;

// Packs the response of a navigation strategy
struct Response {
	Targets M;
	int s;
	Response() : M(), s(0) { }
	Response(Targets M, int s) : M(M), s(s) { }
};

// Base class for navigation strategies
#include <string>
#include <cassert>
struct Strategy {
	string name;
	Strategy(string name) : name(name) { }
	
	// The following function should be overloaded in your strategy
	// Do not change the signature of this function
	virtual
	Response step(unsigned b, const Targets& B, const vector<Targets>& Q)
	{ 
		return Response();
	};
};


// Print a vector's content, e.g. cout << response.M << endl;
template <class T>
std::ostream &operator<<(std::ostream &os, const std::vector<T>& v)
{
	os << "[ ";
	for (const auto& i : v) os << i << ' '; 
	os << "]";
	return os;
}


/****************************************/
//  TODO: IMPLEMENT YOUR STRATEGY HERE  //
/****************************************/
struct AI_MY : public Strategy {
	unsigned C;
	unsigned N;
	
	AI_MY(unsigned C, unsigned N) : Strategy("MY UNNAMED STRATEGY"), C(C), N(N)
	{
	}
	
	Response step(unsigned b, const Targets& B, const vector<Targets>& Q)
	/*	INPUT:
	
		b is an integer 0 <= b < N denoting
		   the current location of the bus.
	
		B is a vector [n1, n2, ..] of
		  the destinations of the passengers
		  currently on the bus
		  (not exceeding the capacity), i.e.
		  nk is the destination of passenger k.
		  The order is that of boarding
		  (provided by this function: see M).
		   No destination is the current position.
		 
		Q is a vector of N vectors, where
		  Q[n] = [t1, t2, ..] is the vector of
		  people currently waiting at station n
		  with destinations t1, t2, ..
		  No destination equals the location,
		  i.e. (t != n) for any t in Q[n].
		 
		The input variable may be modified 
		within this function w/o consequence.
	//*/
	 
	/*	OUTPUT:
	
		The function should
			return Response(M, s)
		where:
		 
		M is a vector of indices M = [i1, i2, .., im]
		  into the vector Q[b] indicating that
		  the people Q[b][i] will board the bus
		  (in the order defined by M).
		  Set M to the empty vector 
		  if no one boards the bus.
		  Note the constraints:
			B.size() + M.size() <= Capacity C,
		  and
			0 <= i < Q[b].size() for each i in M.
		 
		s is either +1, -1, or 0, indicating
		  the direction of travel of the bus
		  (the next station is (b + s) % N).
	//*/
	{
		
		// return the default response
		return Strategy::step(b, B, Q);
	}
};


// STRATEGY: "Always go in the same direction"
#include <numeric>
struct AI_CLOCK : public Strategy {
	unsigned C;
	unsigned N;
	
	AI_CLOCK(unsigned C, unsigned N) : Strategy("Clock"), C(C), N(N)
	{
	}
	
	Response step(unsigned b, const Targets& B, const vector<Targets>& Q)
	{
		// Number of passengers to board
		unsigned n = min(Q[b].size(), C - B.size());
		
		// Passenger selection from Q[b]
		// Take passengers number 0, 1, .., n-1
		Targets M(n);
		std::iota(M.begin(), M.end(), 0);
		
		// Always go in the same direction
		int s = +1;
		
		return Response(M, s);
	}
};


// STRATEGY: Modestly greedy
#include <numeric>
struct AI_GREEDY : public Strategy {
	const char* name;
	unsigned C;
	unsigned N;
	int s;
	
	AI_GREEDY(unsigned C, unsigned N) : Strategy("Modestly greedy"), C(C), N(N)
	{
		// Default direction
		s = +1;
	}
	
	Response step(unsigned b, const Targets& B, const vector<Targets>& Q)
	{
		// Number of passengers to board
		unsigned n = min(Q[b].size(), C - B.size());
		// Passenger selection from Q[b]
		// Take passengers number 0, 1, .., n-1
		Targets M(n);
		std::iota(M.begin(), M.end(), 0);
		
		// No passengers? Continue as before
		if (B.empty() & M.empty()) return Response(M, s);

		// Next passenger's destination
		int t;
		if (!B.empty()) t = B[0]; else t = Q[b][M[0]];

		// Destination relative to the current position
		t = N - 2*((t+N-b) % N);

		// Move towards that destination (modify default direction)
		s = (t > 0 ? +1 : -1);
		
		return Response(M, s);
	}
};




// Function to compute the numerical average of a vector
#include <numeric>
#include <cassert>
template<class V>
double mean(const V& v) {
	assert(!v.empty());
	double sum = std::accumulate(v.begin(), v.end(), 0.0);
	return sum / v.size();
}

// Random integers in the range [A, B]
#include <random>
std::default_random_engine RNG(std::random_device{}());
unsigned randint(unsigned A, unsigned B) {
	std::uniform_int_distribution<unsigned> D(A, B);
	return D(RNG);
}

// This class simulates the system step-by-step.
// Do not modify it.
#include <cassert>
#include <algorithm>
struct World {
	// a = location, b = destination of passenger
	struct News {
		unsigned a, b;
		News(unsigned a, unsigned b) : a(a), b(b) { }
	};

	unsigned C;        // Bus capacity
	unsigned N;        // Number of stations
	unsigned b;        // Bus position
	Targets B;         // Bus passengers' destinations
	vector<Targets> Q; // Queues at stations
	unsigned i;        // Iteration number (i.e. time)
	vector<News> NEWS; // World trajectory record
	
	World(unsigned C, unsigned N) : C(C), N(N)
	{
		rewind();
	}

	void rewind()
	{
		b = 0;
		B = Targets();
		Q = vector<Targets>(N);
		i = 0;
	}
	
	News news() {
		while (NEWS.size() <= i) {
			unsigned a = randint(0, N-1);
			unsigned b = (a + randint(1, N-1)) % N;
			NEWS.push_back(News(a,b));
		}
		
		assert((0 <= i) & (i < NEWS.size()));
		return NEWS[i];
	}
	
	void move(Response res)
	{
		// Passengers mount (in the given order)
		{
			for (auto i : res.M)
				B.push_back(Q[b][i]);
		}
		
		// Remove them from the queue
		{
			// Sort in decreasing order
			Targets M = res.M;
			std::sort(M.rbegin(), M.rend()); 
			
			for (auto i : M)
				Q[b].erase(Q[b].begin() + i);
		}
		
		// Advance bus
		b = (b + (N + res.s)) % N;
		
		// Passengers unmount
		B.erase(std::remove(B.begin(), B.end(), b), B.end());
		
		// Advance time
		this->i ++;
		
		// New person arrives at "a" with destination "b"
		News news = this->news();
		Q[news.a].push_back(news.b);
	}
	
	// Number of people waiting averaged over stations
	double get_w()
	{
		std::vector<unsigned> sizes;
		for (auto q : Q)
			sizes.push_back(q.size());
		
		return mean(sizes);
	}
};


#include <cassert>
#include <numeric>
struct Profiler {
	// Number of iterations (time steps)
	// This will be I ~ 1e6
	static const unsigned I = 100000;
	
	// W[i] = average number of people waiting at time i
	vector<double> W;
	// w = average over time
	double w;

	Profiler(World& wrd, Strategy& nav) : W(), w(0)
	{
		assert((0 < I) & (I <= 1e9));
		
		wrd.rewind();
		assert(wrd.i == 0);
		
		// Main loop
		while(wrd.i < I) {
			// Make read-only references
			const unsigned b = wrd.b;
			const Targets& B = wrd.B;
			const vector<Targets>& Q = wrd.Q;
			wrd.move(nav.step(b, B, Q));
			W.push_back(wrd.get_w());
		}
		
		
		assert(!W.empty());
		w = mean(W);
	}
};


// Helper function for checking rank
bool is_zero(unsigned r) { return (r == 0); }


#include <algorithm>
#include <memory>
//
int main() {

	unsigned C = 3;
	unsigned N = 6;
	
	// http://codereview.stackexchange.com/a/109518
	RNG.discard(700000);

	typedef std::shared_ptr<Strategy> P2S;
	vector<P2S> NAV;
	NAV.push_back(P2S(new AI_MY(C,N)));
	NAV.push_back(P2S(new AI_CLOCK(C,N)));
	NAV.push_back(P2S(new AI_GREEDY(C,N)));
	
	// Next rank to be assigned
	unsigned rank = NAV.size();
	
	// Ranks
	vector<unsigned> R(rank, 0);
	// Score histories
	vector< vector<double> > S(rank);
	
	// Perform the competition
	while (rank) {
		cout << "Number of competitors: " << rank << endl;
		
		// Create a rewindable world
		World wrd(C, N);
		
		// Helper struct
		struct Score {
			unsigned n; // Contestant number
			double v;   // Score value
			Score(unsigned n, double v) : n(n), v(v) { }
			operator double() const { return v; }
		};
		
		// Navigator scores for this round
		// (nonnegative; max score loses)
		vector<Score> K;
		
		for (unsigned n = 0; n < NAV.size(); ++n) {
			if (R[n]) continue;
			Strategy& nav = *NAV[n];
		
			cout << " - Profiling " << nav.name << endl;
			// Profile the navigator on the world
			Profiler report(wrd, nav);
			// Score = average number of people waiting
			double score = report.w;
			// Record score
			K.push_back( Score(n, score) );
			cout << "   *Score for this round: " << score << endl;
		}
		
		assert(!K.empty());
		
		// Find the maximum score
		double max_score = *std::max_element(K.begin(), K.end());
		
		// Rank the losers of this round
		for (auto i : K) {
			// Append the score of the strategy to its history
			S[i.n].push_back(i.v);
			// Take the running average as the score
			S[i.n].back() = mean(S[i.n]);
			
			// Filter losers
			if (i.v < max_score) continue;
			
			// Assign rank to the n-th strategy
			assert(is_zero(R[i.n]));
			R[i.n] = rank;
		}
		
		rank = std::count_if(R.begin(), R.end(), is_zero);
	}
	
	
	cout << "Final ranking:" << endl;
	{
		for (unsigned rank = 1; rank <= R.size(); ++rank) {
			cout << "Rank " << rank << endl;
			for (unsigned n = 0; n != R.size(); ++n) {
				if (R[n] != rank) continue;
				cout << "   " << (NAV[n]->name) << endl;
			}
		}
	}

	// The history of scores of n-th competitor 
	// is available here as S[n]
	cout << "Running average score history:" << endl;
	for (unsigned n = 0; n != S.size(); ++n)
		cout << "   Contestant #" << n << ": " << S[n] << endl;

	return EXIT_SUCCESS;
}


