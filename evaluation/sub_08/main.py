
"""
Author: Michael Weigelt
License: CC BY 4.0
"""

class AI_MY:
    """
    AI class
    """
    name = "MWeigelt"  # Choose strategy name

    def __init__(self, C, N):
        # Capacity of the bus (integer >= 1)
        self.C = C
        # Number of stations (integer >= 2)
        self.N = N
        self.momentum = 0
        self.waiterWeight = 0.1
        
    def dist(self, b, x):
        d1 = x - b
        if d1 < 0:
          d2 = d1 + self.N
        else:
          d2 = d1 - self.N
        if (abs(d1) < abs(d2)):
          d = d1
        else:
          d = d2
        return d
    
    def step(self, b, B, Q):
        
        # Num of passengers boarding here 
        n = min(len(Q[b]), self.C - len(B))
        M = []
        #if filling up bus, let people in and outside bus vote on direction       
        busVote = sum([1.0/self.dist(b,x) for x in B])
        #if we can only pick a subset of people:
        if n == self.C - len(B):
          #tuples of waiting people's targets and their scaled votes (distances)
          waitingTargetWeights = list(zip(Q[b], [1.0/self.dist(b,x) for x in Q[b]], list(range(len(Q[b])))))
          lefts  = sorted([(x,y,z) for (x,y,z) in waitingTargetWeights if y < 0], key = lambda tup: tup[1])
          rights = sorted([(x,y,z) for (x,y,z) in waitingTargetWeights if y > 0], key = lambda tup: tup[1], reverse = True)
          
          # do n with heaviest vote from right weight more than n from left?
          leftVote = sum([y for (x,y,z) in lefts[0:n]])
          rightVote = sum([y for (x,y,z) in rights[0:n]])
          
          if abs(busVote + leftVote) < abs(busVote + rightVote):
            #let right going people board
            boardingIndices = [z for (x,y,z) in rights[0:n]]
            #are there still free spaces? if so, choose from other set
            # among those, prefer ones with far away targets
            if len(boardingIndices) < n:
              remainder = n - len(boardingIndices)
              boardingIndices.extend([z for (x,y,z) in reversed(lefts)][0:remainder])
          else:
            boardingIndices = [z for (x,y,z) in lefts[0:n]]
            #are there still free spaces? if so, choose from other set
            # among those, prefer ones with far away targets
            if len(boardingIndices) < n:
              remainder = n - len(boardingIndices)
              boardingIndices.extend([z for (x,y,z) in reversed(rights)][0:remainder])
          
          M = boardingIndices
          
          
        #if we can pick them all:
        else:
          M = list(range(n))
        
        newcomerDestinations = []
        for i in range(len(M)):
            newcomerDestinations.append(Q[b][M[i]])
        
        destinations = B
        destinations.extend(newcomerDestinations)
        
        distances = []
        for x in destinations:
          d = self.dist(b, x)
          distances.append(d)
        averageDirection = 0
        for x in distances:
          averageDirection += 1.0/x
          
        self.momentum = averageDirection
        
        # add weighted vote of waiting people to average direction
        if len(B) < self.C:
          waiters = [(self.waiterWeight/self.dist(b, i))*len(Q[i]) for i in range(len(Q)) if not i==b and len(Q[i]) > 2] #and len(Q[i]) > 2
          waiterVote = sum(waiters)
          averageDirection += waiterVote
        
        if averageDirection < 0:
          s = -1
        else:
          s = 1
        
        return M, s
