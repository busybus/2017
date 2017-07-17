

// STRATEGY: "Always go in the same direction", aka AI_CLOCK
#include <numeric>
struct AI_MY : public Strategy {
	unsigned C;
	unsigned N;
	
	AI_MY(unsigned C, unsigned N) : Strategy("Clock"), C(C), N(N)
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

