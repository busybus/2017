
// Author: Daniel Kurmann
// License: CC BY 4.0

import java.util.ArrayList;
import java.util.List;

public class AI_MY extends Strategy {

	final public static String name = "Danis Artificial not-Intelligence";
	
	public AI_MY(int C, int N) {
		super(C, N, name);
	}
	
	@Override
	public Response step(int b, List<Integer> B, List<List<Integer>> Q) 
        {
           
                List<Integer> EveryoneOnBus = new ArrayList<>();
                for (int i = 0; i < B.size();i++){EveryoneOnBus.add(B.get(i));}  
              
                List<Integer> M = new ArrayList<>();
                if ((this.C-B.size())<Q.get(b).size()) //Queue is bigger than empty seats
                {
		    int n = this.C - B.size();
                    if (n == 0){return new Response(M, weightedBus(b,B));} //no room, dont let the first passenger board!!
                    int entered=0;
                    for (int i = 1; i<=N/2;i++)
                    {
                        for (int j = 0; j<Q.get(b).size();j++)
                        {
                            if (getDistance(Q.get(b).get(j),b)==i)
                            {
                                M.add(j);
                                EveryoneOnBus.add(Q.get(b).get(j));
                                entered++;
                                if (entered==n)
                                {
                                   return new Response(M, weightedBus(b,EveryoneOnBus));
                                }
                            }
                        }
                    }
                }
                else //there is room for everyone
                {  
		    int n = Q.get(b).size(); 
                    for (int i = 0; i < n; i++)
                    {
                        EveryoneOnBus.add(Q.get(b).get(i));
		        M.add(i);   
                    }
                }
          
                if(B.isEmpty()&& M.isEmpty())
                {
                    return new Response(M, goGetPeople(b,Q));
                }
                return new Response(M,weightedBus(b,EveryoneOnBus)); 
	}


       /**
        * "weighted democracy" this gets called to determine the bus' next move if it has passengers on
        * @param b: bus position
        * @param B: bus passengers
        * @return: -1 (move counterclockwise) or 1 (move clockwise) in case of stalemate, 1 gets chosen
        */
        int weightedBus(int b, List<Integer> B)
        {
            double result=0;
            for (int i=0;i<B.size();i++)
            {
               double d =  (double)getDesiredDirection(b, B.get(i)) / (double)getDistance(b,B.get(i));
               result += d;
            }
            if (result >=0){return 1;}
            else {return -1;}
        }


       /**
        * to determine distance between bus position and to where one person wants to go
        * it just needs two points, it doesnt matter which one is which
        * checks both ways and selects the shorter one
        * @param a position a
        * @param b position b
        * @return distance between a and b
        */
        int getDistance(int a, int b)
        {
            int clockwise=0;
            int target=b;
            while (target!=a)
            {
                
                clockwise++;
                target=(target+1)%(N);
            }
            int counterClockwise=0;
            target=b;
            while (target!=a)
            {
               
                counterClockwise++;
                target=(target-1)%(N);
                if (target<0){target=target+N;}
            }
            if (clockwise-counterClockwise <= 0){return clockwise;}
            else{return counterClockwise;}
        }
   
       /**
        * to determine which way is the shortest between two points
        * checks both ways and determines the shorter way
        * @param busPosition start-position 
        * @param target desired position
        * @return -1 if counterclockwise is shorter, 1 if clockwise is shorter, 1 if distances are equal
        */
        int getDesiredDirection(int busPosition, int target)
        {
            int clockwise=0;
            int b = busPosition;
            while (b!=target)
            {
                clockwise++;
                b=(b+1)%(N);
            }
            int counterClockwise=0;
            b = busPosition;
            while (b!=target)
            {
                counterClockwise++;
                b=(b-1)%(N);
                if (b<0){b=b+N;}
            }
            if (clockwise-counterClockwise <= 0){return 1;}
            else {return -1;}
        }

       /**
        * gets called to determine move direction if the bus is empty
        * checks how many passengers are waiting in both directions at increasing distances
        * as soon as there isnt an equal number waiting at the distance, returns the direction to where 
        * there are more people
        * @param b bus Position
        * @param Q All Queues of all stations
        * @return -1 for counter-clockwise, 1 for clockwise, 1 if theres an equal amount at both sides at every distance
        */
        int goGetPeople(int b, List<List<Integer>> Q)
        {
           int untererIndex;
           for (int i=1;i<=N/2;i++)
           {
              untererIndex=b-i;
              if (untererIndex<0){untererIndex += N;}
              if (Q.get((b+i)%N).size() > Q.get(untererIndex).size()){return 1;}
              if (Q.get((b+i)%N).size() < Q.get(untererIndex).size()){return -1;}
           }
           return 1;
        }
}
