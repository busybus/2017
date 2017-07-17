// Template by verybusybus.wordpress.com

// Author:
// Michał & Jerzy Tyszkiewicz
//    



/*
 Copyright 2017, Michał & Jerzy Tyszkiewicz
 
 BSD 3 clause licence
 
 Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
 
 1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
 
 2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
 
 3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
 
 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <vector>
#include <algorithm>

using namespace std;

// argmax dla wektora dowolnego typu T
template <typename T>
unsigned argmax(const std::vector<T> & v)
{
	unsigned ix_so_far = 0;
	T val_so_far = v[0];
	for (unsigned i = 0; i < v.size(); ++i) {
		if (v[i] > val_so_far) {
			val_so_far = v[i];
			ix_so_far = i;
		}
	}
	
	return ix_so_far;
}

inline int sign(int arg) {
	if (arg == 0)
		return 0;
	return arg > 0 ? 1 : -1;
}

template <typename T>
int mode2(const vector<T> & v)
{
	T sum(0);
	for (T t : v)
		sum += t;
	
	return sign(sum);
}

unsigned rounding(unsigned p, unsigned b, int dir, unsigned N)
{
	return ((dir * (p - b)) + N) % N;
}

// definiuję makro które "zasysa" N i b z kontekstu
#define ROUND(p, dir) (rounding(p, b, dir, N))

struct Passenger
{
	Passenger(unsigned dst,
			  unsigned dep,
			  unsigned stop_id,
			  bool is_real) :
	dst(dst),
	dep(dep),
	stop_id(stop_id),
	is_real(is_real)
	{}
	
	// konstruktor domyślny
	Passenger() : Passenger(0, 0, 0, false) {}
	
	unsigned dst;
	unsigned dep;
	unsigned stop_id;
	bool is_real;
};

/* UWAGA: definiuję skrótowe nazwy typów !!!
 czyli potem w kodzie Queue=vector<vector<unsigned>> i tak dalej
 */
using Queue = vector< vector<unsigned> >;
using Stops = vector< vector<Passenger> >;
using PicksCost = pair<vector<unsigned>, unsigned>;

void update_stops(Stops & stops, const vector<int> & lista, unsigned & expect)
{
	assert(lista.size() == stops.size());
	for (unsigned i = 0; i < lista.size(); ++i) {
		assert((lista[i] == -1) || (lista[i] < stops[i].size()));
		
		if (lista[i] == -1)
	  continue;
		
		Passenger & p = stops[i][lista[i]];
		if (p.is_real && (p.dst > p.dep)) {
			++expect;
			
			p.dst = p.dep + 1;
			p.is_real = false;
		}
	}
	sort(stops[0].begin(),
		 stops[0].end(),
		 [](Passenger i, Passenger j) { return i.dst < j.dst; });
}

struct AI_MY : public Strategy {
public:
	unsigned C;
	unsigned N;
	
private:
	vector<float> penalty;
	vector<float> reward1;
	vector<float> reward2;
	vector<float> reward3;
	int factor;
	int reverse_count;
	int s;
	unsigned limit;
	
	int dir_distance(int from, int to, int dir)
	{
		return (dir * (to - from) + N) % N;
	}
	
	int next(int pos, int dir) {
		return (pos + dir + N) % N;
	}
	
	Stops build_stops3(const Queue & Q, unsigned b, int dir, int pivot)
	{
		Stops stops = Stops(N);
		
		unsigned step_number = 0;
		const unsigned pivot_at_step = dir_distance(b, pivot, dir);
		
		// steps up until pivot point, EXCLUSIVE
		for (int position = b; position != pivot; position = next(position, dir)) {
			int queue_id = 0;
			int steps_until_pivot = pivot_at_step - step_number;
			
			for (unsigned pas_dst : Q[position]) {
				int distance_until_destination = dir_distance(position, pas_dst, dir);
				if (distance_until_destination <= steps_until_pivot) {
					stops[step_number].push_back(
												 Passenger(step_number + distance_until_destination,
														   step_number,
														   queue_id,
														   true
														   )
												 );
				} else {
					
					stops[step_number].push_back(
												 Passenger(N, // zmiana
														   step_number,
														   queue_id,
														   true)
												 );
				}
				
				++queue_id;
			}
			
			stops[step_number].push_back(Passenger(step_number+1, step_number, queue_id, false));
			++step_number;
		}
		
		// steps starting at pivot and further
		for (int position = pivot; step_number < N; position = next(position, -dir)) {
			int queue_id = 0;
			for (unsigned pas_dst : Q[position]) {
				int distance_until_destination = dir_distance(position, pas_dst, -dir);
				if (distance_until_destination + step_number < N) {
					assert(step_number + distance_until_destination < N);
					stops[step_number].push_back(
												 Passenger(step_number + distance_until_destination,
														   step_number,
														   queue_id,
														   true
														   )
												 );
				} else {
					
					stops[step_number].push_back(
												 Passenger(N, // zmiana
														   step_number,
														   queue_id,
														   true)
												 );
				}
				
				
				++queue_id;
			}
			stops[step_number].push_back(Passenger(step_number+1, step_number, queue_id, false));
			++step_number;
		}
		sort(stops[0].begin(),
			 stops[0].end(),
			 [](Passenger i, Passenger j) { return i.dst < j.dst; });
		return stops;
	}
	
	vector<int> list_to_take(const Stops & stops,
							 unsigned start,
							 const vector<float> & reward)
	{
		vector<vector<float>> plan_weights(N+1);
		plan_weights[N].push_back(0.5);
		vector<float> max_weights(N + 2);
		vector<unsigned> max_weight_indexes(N);
		
		for (int stop = 0; stop <= N+1; stop++){max_weights[stop] = -1.;};
		
		for (int stop = N-1; stop >= 0; --stop) {
			for (Passenger traveller : stops[stop]) {
				if (!traveller.is_real) { // wirtualny pasażer
					int ix_max = argmax(max_weights);
					float val_max = max_weights[ix_max];
					plan_weights[stop].push_back(val_max - 0.05 - (ix_max-stop)*0.01);
				} else if ((traveller.dst <= stop) || (traveller.dst >= N)) { // poza zasięg planu
					plan_weights[stop].push_back(0.5);
				} else { // prawdziwy pasażer
					plan_weights[stop].push_back(max_weights[traveller.dst] + reward[stop]);
				}
			}
			
			unsigned max_ix = argmax(plan_weights[stop]);
			max_weights[stop]  = plan_weights[stop][max_ix]; // najlepszy wynik
			max_weight_indexes[stop] = max_ix; // kto go osiaga
		}
		
		Passenger last_in_chain = stops[start][max_weight_indexes[start]];
		vector<int> chain_of_travellers(N);
		
		for (unsigned i = 0; i < N; ++i)
			chain_of_travellers[i] = -1;
		
		chain_of_travellers[start] = max_weight_indexes[start];
		
		for (unsigned stop = start + 1; stop < N - 1; ++stop) {
			if (stop == last_in_chain.dst) {
				chain_of_travellers[stop] = max_weight_indexes[stop];
				last_in_chain = stops[stop][max_weight_indexes[stop]];
			} else {
				chain_of_travellers[stop] = -1;
			}
		}
		
		return chain_of_travellers;
	}
	
	PicksCost List_to_take(Stops stops,
						   const Queue & Q,
						   unsigned b,
						   int dir,
						   int pivot,
						   const vector<float> & reward,
						   const vector<unsigned> & B)
	{
		vector<unsigned> M;
		unsigned expect = 0;
		unsigned n_free_seats = C - B.size();
		
		for (unsigned free_seat = 0; free_seat < n_free_seats; ++free_seat) {
			vector<int> lista = list_to_take(stops, 0, reward);
			if ((lista[0] != -1) && (stops[0][lista[0]].is_real))
				M.push_back(stops[0][lista[0]].stop_id);
			
			update_stops(stops, lista, expect);
		}
		
		const unsigned pivot_at_step = dir_distance(b, pivot, dir);
		
		for (unsigned released_seat : B) {
			// pasażerowie jadący za plan są na marne
			if (dir_distance(b, released_seat, dir) > pivot_at_step) continue;
			
			vector<int> lista = list_to_take(stops, ROUND(released_seat, dir), reward);
			++expect;
			
			update_stops(stops, lista, expect);
		}
		
		return make_pair(M, expect);
	}
	
	PicksCost pool_decision_weights(const Queue & Q,
									const vector<unsigned> & B,
									unsigned b,
									int dir,
									int pivot)
	{
		Stops stops = build_stops3(Q, b, dir, pivot);
		
		PicksCost go1 = List_to_take(stops, Q, b, dir, pivot, reward1, B);
		PicksCost go2 = List_to_take(stops, Q, b, dir, pivot, reward2, B);
		PicksCost go3 = List_to_take(stops, Q, b, dir, pivot, reward3, B);
		
		
		if (go1.second < go2.second && go3.second < go2.second)
			return go2;
		else if (go1.second < go3.second && go2.second < go3.second)
			return go3;
		else
			return go1;
	}
	
	int kier2(unsigned to, unsigned from)
	{
		int exp = N - ((s * (to - from)) % N) * 2;
		return sign(exp);
	}
	
#define KIER(to) (kier2(to, b))
	
public:
	AI_MY(unsigned C, unsigned N):
	Strategy("Graph strategy"),
	C(C),
	N(N),
	penalty(vector<float>(N)),
	reward1(vector<float>(N)),
	reward2(vector<float>(N)),
	reward3(vector<float>(N)),
	factor(4),
	reverse_count(N),
	s(1),
	limit(N)
	{
		for (unsigned i = 0; i < N; ++i)
			penalty[i] = 1. / (float) N;
		
		reward1[0] = 1.6;
		reward1[1] = 1.4;
		reward1[2] = 1.2;
		
		reward2[0] = 1.4;
		reward2[1] = 1.2;
		reward2[2] = 1.2;
		
		reward3[0] = 1.4;
		reward3[1] = 1.2;
		reward3[1] = 1.1;
		
		
		for (unsigned i = 3; i < N; ++i) {
			reward1[i] = 1.;
			reward2[i] = 1.;
			reward3[i] = 1.;
		}
	}
	
	PicksCost max(PicksCost a, PicksCost b) {
		return a.second > b.second ? a : b;
	}
	
	Response step_new(unsigned b, const Targets& B, const vector<Targets>& Q)
	{
		PicksCost best = pool_decision_weights(Q, B, b, 1, next(b, 1));
		s = 1;
		int best_pivot = 1;
		
		for (unsigned pivot = 1; pivot < N; ++pivot) {
			if (pivot == b) continue;
			
			PicksCost left  = pool_decision_weights(Q, B, b,  1, pivot);
			PicksCost right = pool_decision_weights(Q, B, b, -1, pivot);
			
			if (left.second > best.second) {
				best = left;
				s = 1;
				best_pivot = pivot;
			}
			
			if (right.second > best.second) {
				best = right;
				s = -1;
				best_pivot = pivot;
			}
		}
		return Response(best.first, s);
	}
	Response step(unsigned b, const Targets& B, const vector<Targets>& Q) {
		return step_new(b, B, Q);
	}
	
};

