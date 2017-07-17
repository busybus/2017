
// Author: David Ochsner
// License: CC BY 4.0

#pragma once

#include <map>
#include <limits>
#include <random>
#include <cstdint>
#include <cassert>
#include <numeric>
#include <algorithm>
#include <functional>

struct AI_MY : public Strategy {
    using Plan_t = std::uint8_t;
    using Fitness_t = unsigned;
    using Individual = std::pair<Plan_t, Fitness_t>;

	const unsigned C;
	const unsigned N;
    const unsigned planLength;
    Plan_t currPlan;
    
    std::mt19937 gen;
    std::uniform_int_distribution<Plan_t> distr;
    std::function<int()> randNo;
	
	AI_MY(unsigned C, unsigned N) : Strategy("Greedy mGA"),
                                    C(C),
                                    N(N),
                                    planLength(sizeof(Plan_t)*8),
                                    currPlan(0),
                                    gen(42),
                                    distr(0,std::numeric_limits<Plan_t>::max())
	{
        randNo = std::bind(distr, std::ref(gen));    //how to get the type of rng?
        
        //http://codereview.stackexchange.com/a/109518
        gen.discard(700000);
	};
	
	Response step(unsigned currLocation, const Targets& onboardDest, const vector<Targets>& waitingList);
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

    
    //returns the "goodness" of a plan, progressively punishing plans for people on the bus and people waiting at stations
    Fitness_t objectiveFunction(const Plan_t plan,
                                unsigned currLocation,
                                Targets onboardDest,
                                std::vector<Targets> waitingList);
    
    //Assings a fitness-value to the individual, either by finding it in evalHistory or by evaluating the fitness function and adding the result to the evalHistory
    void evalFitness(Individual& a,
                     const unsigned currLocation,
                     const Targets& onboardDest,
                     const std::vector<Targets>& waitingList,
                     std::map<Plan_t,Fitness_t>& evalHistory);
    
    //Do one iteration of a mGA, searching for the optimal plan and returning the best plan found
    Individual mGAIteration(const unsigned currLocation,
                            const Targets& onboardDest,
                            const std::vector<Targets>& waitingList,
                            std::map<Plan_t,Fitness_t>& evalHistory);
    
    //checks whether the individuals in the population have converged towards a good fitness
    bool checkConvergence(const std::vector<Individual>& population);
    
    //Fills up the vector population with random new individuals, reusing old fitness evals
    void makeNewPopulation(std::vector<Individual>& population,
                           const unsigned currLocation,
                           const Targets& onboardDest,
                           const std::vector<Targets>& waitingList,
                           std::map<Plan_t,Fitness_t>& evalHistory);

    //returns an unordered new generation, based on the ordered given population
    std::vector<Individual> makeNewGeneration(const std::vector<Individual>& population,
                                              const unsigned currLocation,
                                              const Targets& onboardDest,
                                              const std::vector<Targets>& waitingList,
                                              std::map<Plan_t,Fitness_t>& evalHistory,
                                              const unsigned crossoverPoint,
                                              const unsigned crossoverType);
    
    //combines the two given plans at the crossoverPoint
    std::pair<Plan_t,Plan_t> makeChildren(const Plan_t a,
                                          const Plan_t b,
                                          const unsigned crossoverPoint);
    
    //returns the positions of people to be taken on the bus
    Targets boardingRule(unsigned nBoarders,
                         unsigned currLocation, const Targets& waiting);
};


Response AI_MY::step(unsigned currLocation,
                     const Targets& onboardDest,
                     const std::vector<Targets>& waitingList)
{
    //run mGA
    unsigned nIterations = 25;
    std::map<Plan_t,Fitness_t> evalHistory;
    std::vector<Individual> bestPlans(nIterations);
    for (unsigned i = 0; i < nIterations; ++i)
    {
        bestPlans[i] = mGAIteration(currLocation, onboardDest, waitingList,evalHistory);
        if (bestPlans[i].second == 0) break;
    }
    
    //determine best NEW plan
    Individual candidate(0,-1);
    for(auto plan : bestPlans)
    {
        if (plan.second < candidate.second) candidate = plan;
    }
    Plan_t oldPlan = currPlan>>1;
    //maybe we found the same plan again?
    if (candidate.first != oldPlan) {
        //update fitness of current Plan
        Individual oldInd(currPlan,0);
        evalFitness(oldInd, currLocation, onboardDest, waitingList, evalHistory);
        //make best plan the currplan
        if (candidate.second < oldInd.second) currPlan = candidate.first;
        else currPlan = oldPlan;
    }
    
    //decide which passengers to pick up
    //HAS TO BE DONE THE SAME WAY AS IN THE OBJECTIVE FUNCTION
    unsigned nNewPassengers = std::min(waitingList[currLocation].size(), C-onboardDest.size());
    
    Targets passengersToPickUp = boardingRule(nNewPassengers, currLocation, waitingList[currLocation]);
    
    //map 0,1 to -1,1
    int nextStep = 2* (currPlan&1) - 1;
    
    return Response(passengersToPickUp, nextStep);
}


AI_MY::Individual AI_MY::mGAIteration(const unsigned currLocation,
                                      const Targets& onboardDest,
                                      const std::vector<Targets>& waitingList,
                                      std::map<Plan_t,Fitness_t>& evalHistory)
{
    const unsigned maxGenerations = 50;
    std::vector<Individual> population(5);

    makeNewPopulation(population, currLocation, onboardDest, waitingList, evalHistory);
    
    for (unsigned i = 0; i < maxGenerations; ++i)
    {
        //sort in ascending order of their fitness value, for easier convergence check & recombination
        std::sort(population.begin(),population.end(),[](Individual a, Individual b) {
            return a.second < b.second;
        });
        
        if (checkConvergence(population))
        {
            break;
        }
        
        //determine parameters for crossover
        const unsigned crossoverPoint = double(gen()) / (gen.max()+1) * (planLength-1) + 1;
        //how to recombine the four plans
        const unsigned crossoverType = double(gen()) / (gen.max()+1) * 3;
        
        population = makeNewGeneration(population, currLocation, onboardDest, waitingList, evalHistory, crossoverPoint, crossoverType);
    }
    
    std::sort(population.begin(),population.end(),[](Individual a, Individual b) {
        return a.second < b.second;
    });
    
    return population[0];
}


void AI_MY::makeNewPopulation(std::vector<Individual>& population,
                              const unsigned currLocation,
                              const Targets& onboardDest,
                              const std::vector<Targets>& waitingList,
                              std::map<Plan_t,Fitness_t>& evalHistory)
{
    assert(population.size() == 5);
    for(auto& popMember : population)
    {
        //choose random plan
        popMember.first = randNo();
        
        evalFitness(popMember, currLocation, onboardDest, waitingList, evalHistory);
    }
}


std::vector<AI_MY::Individual> AI_MY::makeNewGeneration(const std::vector<Individual>& population,
                                                        const unsigned currLocation,
                                                        const Targets& onboardDest,
                                                        const std::vector<Targets>& waitingList,
                                                        std::map<Plan_t,Fitness_t>& evalHistory,
                                                        const unsigned crossoverPoint,
                                                        const unsigned crossoverType)
{
    //assuming sorted population - population[0] has best fitness
    auto newGeneration = population;
    
    //case:     crossover between:
    //  0           0&1, 2&3
    //  1           0&2, 1&3
    //  2           0&3, 1&2
    
    //set pointers according to crossover-type
    const Plan_t *parentA = nullptr, *parentB = nullptr, *parentC = nullptr, *parentD = nullptr;
    switch(crossoverType)
    {
        case 0: parentA = &population[0].first;
                parentB = &population[1].first;
                parentC = &population[2].first;
                parentD = &population[3].first;
                break;
        case 1: parentA = &population[0].first;
                parentB = &population[2].first;
                parentC = &population[1].first;
                parentD = &population[3].first;
                break;
        case 2: parentA = &population[0].first;
                parentB = &population[3].first;
                parentC = &population[1].first;
                parentD = &population[2].first;
    }
    
    assert(parentA != nullptr && parentB != nullptr && parentC != nullptr && parentD != nullptr);
    
    //Get the children
    std::pair<Plan_t,Plan_t> children = makeChildren(*parentA, *parentB, crossoverPoint);
    newGeneration[1].first = children.first;
    newGeneration[2].first = children.second;
    children = makeChildren(*parentC, *parentD, crossoverPoint);
    newGeneration[3].first = children.first;
    newGeneration[4].first = children.second;
    
    for (unsigned i = 1; i < 5; ++i)
    {
        evalFitness(newGeneration[i], currLocation, onboardDest, waitingList, evalHistory);
    }
    
    return newGeneration;
}


std::pair<AI_MY::Plan_t,AI_MY::Plan_t> AI_MY::makeChildren(const Plan_t a,
                                                           const Plan_t b,
                                                           const unsigned crossoverPoint)
{
    //Recombine the given plans at the crossover point
    assert(crossoverPoint < planLength && crossoverPoint > 0);
    std::pair<Plan_t, Plan_t> children;
    
    //Example with plans 10101,01010 and crossover point 2
    
    //Bitmasks to extract the wanted bits
    //Care, c++ saves the least significant bit at the rightmost position,
    //so bitshift works counterintuitive
    Plan_t leftMask = Plan_t(-1) >> crossoverPoint;                 // 111111 >> 2 == 11100
    Plan_t rightMask = Plan_t(-1) << (planLength-crossoverPoint);   // 111111 << 2 == 00111
    
    
    //10101 & 11100 == 10100
    //01010 & 00111 == 00010
    //10100 | 00010 == 10110
    children.first = (a & leftMask) | (b & rightMask);
    
    //10101 & 00111 == 00101
    //01010 & 11100 == 01000
    //00101 | 01000 == 01101
    children.second = (a & rightMask) | (b & leftMask);
    
    return children;
}


void AI_MY::evalFitness(Individual& a,
                        const unsigned currLocation,
                        const Targets& onboardDest,
                        const std::vector<Targets>& waitingList,
                        std::map<Plan_t,Fitness_t>& evalHistory)
{
    auto search = evalHistory.find(a.first);
    if (search != evalHistory.end())    //plan has already been evaluated
    {
        a.second = search->second;
    }
    else    //plan hasn't been evaluated for this situation
    {
        a.second = objectiveFunction(a.first, currLocation, onboardDest, waitingList);
        evalHistory[a.first] = a.second;
    }
}



AI_MY::Fitness_t AI_MY::objectiveFunction(const Plan_t plan,
                                          unsigned currLocation,
                                          Targets onboardDest,
                                          std::vector<Targets> waitingList)
{
    assert(currLocation < N);
    
    //Variables to keep track of the fitness value
    Fitness_t fitness = 0;
    unsigned peopleWaiting = 0;
    for(auto pos : waitingList)
    {
        peopleWaiting += pos.size();
    }
    
    //work through plan, analogous to a move() in World
    for (unsigned k = 0; k < planLength; ++k)
    {
           
        //how many passengers can be taken aboard
        unsigned nNewPassengers = std::min(waitingList[currLocation].size(), C-onboardDest.size());
        
        //determine who comes onboard
        auto newPassengers = boardingRule(nNewPassengers, currLocation, waitingList[currLocation]);
            
        //passengers mount
        for (auto i : newPassengers)
        {
            onboardDest.push_back(waitingList[currLocation][i]);
        }
        
        //erase newly boarded passengers from waiting list
        std::sort(newPassengers.rbegin(), newPassengers.rend()); 
			
        for (auto i : newPassengers)
        {
            waitingList[currLocation].erase(waitingList[currLocation].begin() + i);
        }

        //advance bus
        currLocation = (currLocation + (N + 2*((plan>>k)&1)-1)) % N; //2*((plan>>i)&1)-1 is either +1 or -1 depending on the i-th bit in plan
        
        //passengers unmount
        onboardDest.erase(std::remove(onboardDest.begin(), onboardDest.end(), currLocation), onboardDest.end());
        
        //updating fitness value
        peopleWaiting -= nNewPassengers;
        fitness += k * peopleWaiting;
        fitness += k * onboardDest.size();
    }
    
    return fitness;
}


bool AI_MY::checkConvergence(const std::vector<Individual>& population)
{
    //expects a sorted container containing Individuals
    Fitness_t bestFitness = population.begin()->second;
    
    
    Fitness_t worstFitness = (population.end()-1)->second;
    //the objective-function-values are "close"
    if (double(worstFitness - bestFitness)/bestFitness < .05) {
        return true;
    }

    return false;
    
    //Different possibility for convergence-check: how many bits differ in best/worst
    //~ Plan_t difference = population.begin()->first ^ population.back().first;
    //~ unsigned diffCount = 0;
    //~ while (difference = difference >> 1)
    //~ {
        //~ if (difference & 1) ++diffCount;
    //~ }
    
    //~ if (diffCount < 2) return true;
    //~ return false;
    
    //Different possibility for convergence-check: only stop when best and worst plan ere identical
    //~ Fitness_t bestFitness = population.begin()->second;
    //~ Fitness_t worstFitness = (population.end()-1)->second;
    //~ if (worstFitness - bestFitness) return true;
    //~ return false;
}

Targets AI_MY::boardingRule(unsigned nBoarders,
                            unsigned currLocation,
                            const Targets& waiting)
{
    //Take the last nBoarders for faster vector-manipulation in the objective function
    Targets peopleBoarding(nBoarders);
    std::iota(peopleBoarding.begin(), peopleBoarding.end(), waiting.size()-nBoarders);
    
    return peopleBoarding;
}

//Boarding rule preferring people with short distance to their destination
//~ Targets AI_MY::boardingRule(unsigned nBoarders,unsigned currLocation, const Targets& waiting)
//~ {
    //~ Targets peopleBoarding(nBoarders);
    
    //~ //naive implementation, look at https://en.wikipedia.org/wiki/Selection_algorithm
    //~ std::vector<std::pair<unsigned,unsigned> > pickupOrder(waiting.size()); //pair<posInwaiting,distToDest>
    //~ //create vector of pos/dist pairs
        //~ //std::cout << "      Deciding who to take on at pos "  << currLocation << "(Longest way: " << double(N)/2 << std::endl;
    //~ for (unsigned i = 0; i < waiting.size(); ++i)
    //~ {
        //~ pickupOrder[i].first = i;
        //~ if (waiting[i] != currLocation)
        //~ {
            //~ pickupOrder[i].second = double(N)/2 - std::abs(std::abs(int(currLocation - waiting[i])) - double(N)/2);
        //~ }
        //~ else
        //~ {
            //~ pickupOrder[i].second = 2;  //if you're on waiting on your destination, you'll have to ride at least 2 timesteps to return back
        //~ }
            //~ //std::cout << "      Passenger  wanting to go to " << waiting[i] << " has to travel for " << pickupOrder[i].second << " steps" << std::endl;
    //~ }
    
    //~ //sort in ascending order
    //~ std::sort(pickupOrder.begin(),pickupOrder.end(),[](Individual a, Individual b) {
        //~ return a.second < b.second;
    //~ });
    
    //~ //take passengers with lowest distance
    //~ for (unsigned i = 0; i < nBoarders; ++i)
    //~ {
        //~ peopleBoarding[i] = pickupOrder[i].first;
    //~ }
    
    //~ return peopleBoarding;
//~ }
