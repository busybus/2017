#!/usr/bin/env python3

"""
Author: Rui Viana
License: CC BY 4.0
"""

from random import randint
from numpy import mean
from collections import namedtuple

#  Section 0: Classes
#  ------------------
# pylint:disable=C0103,R0201,R0903,R0913,W0702,W0703

Carry_Option = namedtuple("Carry_Option","from_idx to_idx score")
Passenger = namedtuple("Passenger", "from_st to_st qidx carry_options chosen_option")

class AI_MY:
	name = "Rui's AI"
	def __init__(self, C, N):
		self.C = C
		self.N = N
		self.lastDir = 1
		self.path_len = int(self.N * 1.25)
		disc = 1.02
		self.drop_ws = []
		for idx in range(self.path_len):
			self.drop_ws.append(1.0-(1.0-pow(disc,-idx))/(1.0-pow(disc,-self.path_len+1)))

	def evaluate_path(self, B, Q, path):
		occupations = [0]*self.path_len # how many people in the bus at the time we leave the i'th station
		drops = [0]*self.path_len # how many people we drop into the i'th station
	 
		for passenger in B: # account for all the passengers already in the bus
			for pidx,p in enumerate(path):
				if p == passenger:
					drops[pidx] += 1
					break
				occupations[pidx] += 1
 
 		# make a list of all the queued passengers, and when we'd pick them up and drop them off
		passengers = []
		for from_st, stQ in enumerate(Q):
			visit_pathidxs = [idx for idx, st in enumerate(path) if st==from_st]  # find out when the path visits this station
			for qidx, to_st in enumerate(stQ):
				carry_options = []
				for from_idx in visit_pathidxs:
					after_path = path[from_idx:]
					# after pick up the path must pass by the drop off station
					try: to_idx = after_path.index(to_st)
					except ValueError: break  # if not found, then no point in checking latter visits, so break out
					carry_options.append(Carry_Option(from_idx=from_idx, to_idx=from_idx+to_idx, score=to_idx*self.path_len+from_idx))
 
				if len(carry_options)==0: continue
				carry_options.sort(key=lambda x: x.score)
				passengers.append(Passenger(from_st=from_st, to_st=to_st, qidx=qidx, carry_options=carry_options, chosen_option=[None]))
 
		passengers.sort(key=lambda x: x.carry_options[0].score) # sort based on how long they'll stay on the bus
 
 		# add passengers one at a time until the bus is full, or we run out of passengers to add
		picked = []
		for passenger in passengers:
			for co in passenger.carry_options:
				if not all([occupations[x] < self.C for x in range(co.from_idx, co.to_idx)]): continue
				# we're able to carry this passenger
				for x in range(co.from_idx, co.to_idx): occupations[x] += 1
				passenger.chosen_option[0] = co
				picked.append(passenger)
				break

		# for passengers that were picked, can we pick them up even earlier? if so, do that
		for passenger in picked:
			for co in passenger.carry_options:
				if co.from_idx >= passenger.chosen_option[0].from_idx: continue
				if not all([occupations[x] < self.C for x in range(co.from_idx, passenger.chosen_option[0].from_idx)]): continue
				for x in range(co.from_idx, passenger.chosen_option[0].from_idx): occupations[x] += 1
				passenger.chosen_option[0] = co

		# build the drops and M arrays
		M = []
		for passenger in picked:
			drops[passenger.chosen_option[0].to_idx] += 1
			if passenger.chosen_option[0].from_idx == 0: M.append(passenger.qidx)

		score = 0.0
		for idx,d in enumerate(drops): score += self.drop_ws[idx] * d
		return score, sorted(M)
 
	def step(self, b, B, Q):
		options = []
		for direction in [self.lastDir, self.lastDir*(-1)]:
			path = [(b+x*direction+self.N*10)%self.N for x in range(self.path_len)]
			path_score, path_M = self.evaluate_path(B, Q, path)
			if direction == self.lastDir: path_score += 0.2
			options.append([path, path_score, path_M])
 
			max_pivot_dist = int(0.35*self.N) if direction == self.lastDir else 1
			for pivot_dist in range(1,max_pivot_dist+1):
				path2 = [(b+x*direction+self.N*10)%self.N for x in range(pivot_dist+1)]
				path2 += list(reversed(path2[:-1]))
				path2 += [(b-x*direction+self.N*10)%self.N for x in range(1,self.path_len-len(path2)+1)]
				path_score, path_M = self.evaluate_path(B, Q, path2)
				if direction == self.lastDir: path_score -= 0.08
				options.append([path2, path_score, path_M])
	 
		options.sort(reverse = True, key = lambda x: x[1])
		top_path = options[0][0]
		self.lastDir = (top_path[1] - top_path[0] + 1 + self.N )%self.N - 1
		return options[0][2], self.lastDir

class AI_CLOCK:
	"""
	'Always go in the same direction' strategy
	"""
	name = "Clock"

	def __init__(self, C, N):
		self.C = C
		self.N = N

	def step(self, b, B, Q):
		"""
		Calculates one step.
		"""
		# Number of passengers to board
		n = min(len(Q[b]), self.C - len(B))
		# Passenger selection from Q[b]:
		# Take passengers number 0, 1, .., n-1
		M = list(range(n))

		# Always go in one direction
		s = +1

		return M, s


class AI_GREEDY:
	"""
	'Modestly greedy' strategy
	"""
	name = "Modestly greedy"

	def __init__(self, C, N):
		self.C = C
		self.N = N
		self.s = +1

	def step(self, b, B, Q):
		"""
		Calculates one step.
		"""
		# Number of passengers to board
		n = min(len(Q[b]), self.C - len(B))
		# Passenger selection from Q[b]
		M = list(range(n))

		# No passengers? Continue as before
		if (not B) and (not M):
			return [], self.s

		# Next passenger's destination
		if len(B):
			t = B[0]
		else:
			t = Q[b][M[0]]

		# Destination relative to the current position
		t = self.N - 2 * ((t - b + self.N) % self.N)

		# Move towards that destination (modify default direction)
		self.s = (+1) if (t > 0) else (-1)

		return M, self.s


class World:
	"""
	Simulates the system step by step.
	Do not change this class.
	"""
	def __init__(self, C, N):
		self.C = C         # Bus capacity
		self.N = N         # Number of stations
		self.b = None      # Bus position
		self.B = None      # Bus passengers' destinations [list]
		self.Q = None      # Queues at stations [list of list]
		self.i = None      # Iteration number (i.e. time)
		self.NEWS = [None] # World trajectory record [list of tuple/None]
		self.rewind()

	def rewind(self):
		"""
		Rewinds the world.
		"""
		self.b = 0
		self.B = []
		self.Q = [[] for _ in range(self.N)]
		self.i = 0

	def news(self):
		"""
		Creates news if necessary.

		Returns:
			(a, b): a person arrives at "a" with destination "b".
		"""
		# Create news if necessary
		while len(self.NEWS) <= self.i:
			# New person arrives at "a" with destination "b"
			a = randint(0, self.N - 1)
			b = (a + randint(1, self.N - 1)) % self.N
			self.NEWS.append((a, b))
		assert 0 <= self.i < len(self.NEWS)
		return self.NEWS[self.i]

	def look(self):
		"""
		Returns a copy of (b, B, Q).
		"""
		return self.b, self.B[:], [q[:] for q in self.Q]

	def board1(self, m):
		'''
		Board one passenger
		m is an element of M, see move(...)
		'''
			
		self.B.append(self.Q[self.b][m])
		self.Q[self.b].pop(m)
	
	def move(self, M, s):
		"""
		Performs the move indicated by an AI.

		Args:
			M (:obj: `list` of int): is a list of indices M = [i1, i2, .., im]
				into the list Q[b] indicating that the people Q[b][i] will board
				the bus (in the order defined by M).
				Set M = [] if no one boards the bus.
				Note the constraints:
					len(B) + len(M) <= Capacity C,
				and
					0 <= i < len(Q[b]) for each i in M.
			s (int): is either +1, -1, or 0, indicating the direction of travel
				of the bus (the next station is (b + s) % N).
		"""
		# Check consistency from time to time
		if randint(0, 100) == 0:
			self.check_consistency(self.C, self.N, self.b, self.B, self.Q, M, s)

		# Passengers mount (in the given order)
		# and are removed from the queue
		for m in sorted(M, reverse=True):
			self.board1(m)

		# Advance bus
		self.b = (self.b + (self.N + s)) % self.N

		# Passengers unmount
		self.B = [p for p in self.B if p != self.b]

		# Advance time
		self.i += 1

		assert self.news() is not None
		# New person arrives at "a" with destination "b"
		a, b = self.news()
		# Queue in the new person
		self.Q[a].append(b)

	def get_w(self):
		"""
		Returns:
			Number of people waiting in queue, averaged over the stations.
		"""
		return mean([len(q) for q in self.Q])

	@staticmethod
	def check_consistency(C, N, b, B, Q, M, s):
		"""
		Checks consistency of the input.
		"""

		# 0.
		# C is an integer >= 1
		# N is an integer >= 2

		assert isinstance(C, int) and (C >= 1)
		assert isinstance(N, int) and (N >= 2)

		is_station = lambda n: isinstance(n, int) and (0 <= n < N)

		# 1.
		# b is an integer 0 <= b < N denoting
		#   the current location of the bus.

		assert is_station(b)

		# 2.
		# B is a list [n1, n2, ..] of
		#   the destinations of the passengers
		#   currently on the bus
		#   (not exceeding the capacity), i.e.
		#   nk is the destination of passenger k.
		#   The order is that of boarding
		#   (provided by this function: see M).
		#   No destination is the current position.

		assert isinstance(B, list)
		assert all(is_station(n) for n in B)
		assert all((n != b) for n in B)

		# 3.
		# Q is a list of N lists, where
		#   Q[n] = [t1, t2, ..] is the list of
		#   people currently waiting at station n
		#   with destinations t1, t2, ..
		#   No destination equals the location,
		#   i.e. (t != n) for any t in Q[n].

		assert isinstance(Q, list)
		assert len(Q) == N
		assert all(isinstance(q, list) for q in Q)
		assert all(all(is_station(t) for t in q) for q in Q)
		assert all(all((t != n) for t in q) for n, q in enumerate(Q))

		# 4.
		# M is a list of indices M = [i1, i2, .., im]
		#   into the list Q[b] indicating that
		#   the people Q[b][i] will board the bus
		#   (in the order defined by M).
		#   Set M = [] if no one boards the bus.
		#   Note the constraints:
		#     len(B) + len(M) <= Capacity C,
		#   and
		#     0 <= i < len(Q[b]) for each i in M.

		assert isinstance(M, list)
		assert all(isinstance(i, int) for i in M)
		assert all((0 <= i < len(Q[b])) for i in M)
		assert len(B) + len(M) <= C

		# 5.
		# s is either +1, -1, or 0, indicating
		#   the direction of travel of the bus
		#   (the next station is (b + s) % N).

		assert isinstance(s, int)
		assert (s in [-1, 0, 1])

class Profiler:
	"""
	Runs the systems with a particular strategy "nav".
	"""

	# Number of iterations (time steps)
	# This will be I ~ 1e6
	I = 1000000

	def __init__(self, wrd, nav):
		# W[i] = average number of people waiting at time i
		self.W = []
		# w = average over time
		self.w = None

		assert 0 < self.I <= 1e9

		wrd.rewind()
		assert wrd.i == 0

		# Main loop
		while wrd.i < self.I:
			wrd.move(*nav.step(*wrd.look()))
			self.W.append(wrd.get_w())

		assert len(self.W)
		self.w = mean(self.W)

# Helper function
def get_name(nav):
	"""
	Args:
		nav (:obj: AI_*): the Strategy nav.
	Returns:
		nav.name (str): the name of a nav or "Unknown".
	"""
	try:
		return nav.name
	except:
		return "Unknown"

def main():
	"""
	Main
	"""

	#  Section 1: Initialize candidates
	#  --------------------------------

	# Bus capacity
	C = 10  # This will be around 10
	# Number of stations
	N = 20  # This will be around 20

	print("1. Initializing navigators")

	# Competing navigation strategies
	NAV = []
	NAV.append(AI_MY(C, N))
	NAV.append(AI_CLOCK(C, N))
	NAV.append(AI_GREEDY(C, N))


	#  Section 2: Profile candidates
	#  -----------------------------


	print("2. Profiling navigators")

	# Ranks
	R = [None for _ in NAV]
	# Score histories
	S = [[] for _ in NAV]

	# While some ranks are undecided
	while [r for r in R if r is None]:
		rank = sum((r is None) for r in R)
		print("Number of competitors:", rank)

		# Create a rewindable world
		wrd = World(C, N)

		# Navigator scores for this round
		# (nonnegative; max score loses)
		K = []

		for n, nav in enumerate(NAV):
			if R[n] is not None:
				continue

			print(" - Profiling:", get_name(nav))
			try:
				# Profile the navigator on the world
				report = Profiler(wrd, nav)
				# Score = average number of people waiting
				score = report.w
				# Record score
				K.append((n, score))
				print("   *Score for this round:", score)
			except Exception as err:
				R[n] = rank
				print("   *Error:", err)

		# Rank the losers of this round
		for n, s in K:
			if s == max(s for n, s in K):
				R[n] = rank
			S[n].append(s)


	#  Section 3: Summary of results
	#  -----------------------------


	print("3. Final ranking:")

	for r in sorted(list(set(R))):
		print("  ", r, [get_name(NAV[i]) for i, rr in enumerate(R) if r == rr])


	# The history of scores of n-th competitor
	# is available here as S[n]
	print("Score history:")
	for n, H in enumerate(S):
		print("   Contestant #{0}:".format(n), H)

	# (Un)comment the following line for the score history plot
	"""
	import matplotlib.pyplot as plt
	for s in S :
		plt.plot(s, '-x')
	plt.yscale('log')
	plt.xlabel('Round')
	plt.ylabel('Score (less is better)')
	plt.legend([get_name(nav) for nav in NAV], numpoints=1)
	plt.show()
	#"""

if __name__ == "__main__":
	main()