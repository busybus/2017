"""
Authors: Johan Särnbratt and Olof Düsterdieck
License: CC BY 4.0
"""

class AI_MY :
    """
    AI class
    """

    def __init__(self, C, N, far=15, safe=0.7, weight = 0.94, turnLength = 3, name=None):
        # Capacity of the bus (integer >= 1)
        self.C = C
        # Number of stations (integer >= 2)
        self.N = N
        self.s = 1
        self.debug = False
        if name == None:
            self.name = 'smart turner ' + str(far) + ' ' + str(turnLength)
        else:
            self.name = name
        #far decides how many steps into the future the AI will simulate
        self.far = far
        #The AI will score some different decisions, safe is a score added to "not turning" because it is underscored by the algorithm
        self.safe = safe
        
        # weight is the exponential drop off given to the value of future picked up passengers
        self.weight = weight
        
        #AI simulates scores for turning at most turnLength steps in the future
        self.turnLength = turnLength
        
		#distance from bus to station in following direction
    def dist(self, b, station, direction):
        tempDist = direction*(station-b)
        if tempDist >= 0:
            return tempDist
        return self.N+tempDist
        
    #distance from bus to station if you change direction at turnStation
    def turnDist(self, b, station, direction, turnStation):
    		#same as dist but bus turns around at turnStation
        if self.dist(b,station,direction) > self.dist(b,turnStation,direction):
            return self.dist(b,turnStation,direction)+self.dist(turnStation,station,-direction)
        return self.dist(b,station,direction)
		
    def step(self, b, B, Q):
        cap = self.C - len(B)
        
        #calculate score for continuing in same direction as earlier
        bestEq, bestM = self.packPass(b, B, Q, self.s, self.far) #dont turn
        bestEq += self.safe
        bestS = self.s
        
        #calculate score for change direction immediately
        tempEq, turnM = self.packPass(b, B, Q, -self.s, self.far)
        #Save the best of the scores so far
        if tempEq > bestEq:
            bestEq = tempEq
            bestM = turnM
            bestS = -self.s
        
        #calculate scores for turning around after going forward t steps
        for t in range(1,1+self.turnLength):
            tempEq, tempM = self.packTurn(b, B, Q, self.s, self.far, t)
            #save the best score sofar
            if tempEq > bestEq:
                bestEq = tempEq
                bestM = tempM
                bestS = self.s
        
        self.s = bestS
        return bestM, self.s

    #calculate equity for turning around at station turnAt
    # b - bus position
    # B - passengers on bus
    # Q - qued passengers
    # direction - currently traveling values: 1, -1
    # far - how many steps too look ahead: values: 1-20 (usually around 15)
    # turnAt - station to turn around at
    
    #this function will simulate the bus traveling to station turnAt and then turning around and continuing to travel in the opposite direction.
    #During this time the algorithm will keep the bus as full as possible.
    #The passengers are sorted according to first "passenger travel distance" and second "distance from bus to passenger"
    #A score is calculated based on how soon the bus will pick up a passenger
    def packTurn(self, b, B, Q, direction, far, turnAt):
        turnStation = (b+direction*turnAt)%self.N
        #passPerStep is how many passengers is still in the bus after i steps
        passPerStep = [len(B)]
        for i in range(1, far):
            passPerStep.append(passPerStep[-1]-len([g for g in B if i == self.turnDist(b, g, direction, turnStation)]))
        
        passengers = []
        #collect all passengers in a list of tuples
        #(passenger travel dist, dist until pick up passenger, ind)
        for station in range(0, self.N):
            for ind, p in enumerate(Q[station]):
                if self.dist(b, station, direction)+self.dist(station, p, direction) <= turnAt%self.N:
                    pdist = self.dist(station, p, direction)
                    stationDist = self.dist(b, station, direction)
                else:
                    pdist = self.dist(station, p, -direction)
                    stationDist = self.dist(b, turnStation, direction)+self.dist(turnStation, station, -direction)
                if stationDist < far:
                    passengers.append((pdist, stationDist, ind))
        passengers = sorted(passengers)
        M = []
        eq = 0
        #o through the sorted list of passengers, greedily add every passenger that fits 
        for pas in passengers:
            if max(passPerStep[pas[1]:(pas[1]+pas[0])]) < self.C:
                #If passenger is picked up update the score
                eq = eq+self.weight**pas[1]
                if pas[1] == 0:
                    #If passenger is on current station, add it to list of passengers to pick up
                    M.append(pas[2])
                for a in range(pas[1],min(pas[1]+pas[0],len(passPerStep))):
                    passPerStep[a] = passPerStep[a]+1
        
        return (eq, M)
    #packPass does the same thing as packTurn without turning around anywhere
    def packPass(self, b, B, Q, direction, far):
        passPerStep = [len(B)]
        for i in range(1, far):
            passPerStep.append(passPerStep[-1]-len([g for g in B if g == (b+i*direction)%self.N]))
        
        passengers = []
        #collect all passengers in a list of tuples
        #(distance, start, ind?)
        for i in range(0, far):
            station = (b+i*direction)%self.N
            for ind, p in enumerate(Q[station]):
                passengers.append((self.dist(station, p, direction), i, ind))
        passengers = sorted(passengers)
        M = []
        eq = 0
        
        for pas in passengers:
            if max(passPerStep[pas[1]:(pas[1]+pas[0])]) < self.C:
                eq = eq+self.weight**pas[1]
                if pas[1] == 0:
                    M.append(pas[2])
                for a in range(pas[1],min(pas[1]+pas[0],len(passPerStep))):
                    passPerStep[a] = passPerStep[a]+1
        
        return (eq, M)
