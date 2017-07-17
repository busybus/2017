
// Author: RA
// Wraps a custom strategy, to be called from python driver
// Compile with g++ -std=c++11 main_io.cpp


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



#include "./AI_MY.hpp"



// Function to compute the numerical average of a vector
#include <numeric>
#include <cassert>
template<class V>
double mean(const V& v) {
	assert(!v.empty());
	double sum = std::accumulate(v.begin(), v.end(), 0.0);
	return sum / v.size();
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

	unsigned C;		// Bus capacity
	unsigned N;		// Number of stations
	unsigned b;		// Bus position
	Targets B;		 // Bus passengers' destinations
	vector<Targets> Q; // Queues at stations
	unsigned i;		// Iteration number (i.e. time)
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



#include <sstream>
#include <fstream>
#include <string>
#include <iterator>
int main() {
	string filename = "busybusio";

	// Stage 1a: Input parameters

	unsigned C = 0, N = 0, I = 0;
	{
		std::ifstream fi(filename + ".in");
		assert(!fi.eof());
		std::string line;
		std::getline(fi, line);
		std::istringstream is(line);
		assert(!is.eof()); is >> C;
		assert(!is.eof()); is >> N;
		assert(!is.eof()); is >> I;
		assert(is.eof());
		fi.close();
	}

	assert((0 < I) & (I <= 1e9));

	// Stage 1b: Input "news"

	World wrd(C, N);
	{
		std::ifstream fi(filename + ".in");
		std::string line;
		std::getline(fi, line);

		// Note that the length of NEWS should be at least (I+1)
		for (unsigned i = 0; i <= I; ++i) {
			assert(!fi.eof());
			std::getline(fi, line);
			std::istringstream is(line);
			unsigned a, b;
			assert(!is.eof()); is >> a;
			assert(!is.eof()); is >> b;
			assert(is.eof());

			assert((0 <= a) & (a < N));
			assert((0 <= b) & (b < N));
			wrd.NEWS.push_back(World::News(a, b));
		}

		fi.close();
	}

	// Stage 2: Simulate and output

	{
		std::ofstream fo(filename + ".out");

		AI_MY nav(C, N);

		wrd.rewind();
		assert(wrd.i == 0);

		// Main loop
		while(wrd.i < I) {
			// Make read-only references
			const unsigned b = wrd.b;
			const Targets& B = wrd.B;
			const vector<Targets>& Q = wrd.Q;
			Response r = nav.step(b, B, Q);

			fo << r.s;
			for (auto m : r.M) fo << " " << m;
			fo << endl;

			wrd.move(r);
		}

		fo.close();
	}

	return EXIT_SUCCESS;
}
