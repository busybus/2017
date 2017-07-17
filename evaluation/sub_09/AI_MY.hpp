// Author: Linus Groner
// License: CC BY 4.0

struct AI_PENDEL : public Strategy {
	unsigned C;
	unsigned N;
	unsigned state;
	unsigned ctr;
	unsigned period;
	AI_PENDEL(unsigned C, unsigned N,unsigned period=38) : Strategy("PENDEL "+to_string(period)), C(C), N(N),period(period), 
	ctr(0),state(1)
	{
	};

	void switchState(){
		if(state==1)
			state=-1;
		else
			state=1;
	};


	int relDist(unsigned b, unsigned N, unsigned pos){
		int N2 = N/2;
		int shift = static_cast<int>(N2 - b);
		
		int resPos = pos;
		resPos += shift;
		if (resPos<0)
			resPos = N+resPos;
		else
			resPos %=(int)N;
		resPos -= N2;
		return (!(N%2) && resPos==-N2?-resPos:resPos);	
	}
	Response step(unsigned b, const Targets& B, const vector<Targets>& Q)
	/*
	 * Go same direction for this->period number of steps. Take as many passengers waiting for same direction, fill up with others.
	 */	

	{
		unsigned N = Q.size();
		if(ctr == 0)
		{
			switchState();
			ctr = period;
		}else{ctr--;}

		vector<unsigned> maxFillLeft = vector<unsigned>();
		vector<unsigned> maxFillRight = vector<unsigned>();
		unsigned ctrLeft = 0;
		unsigned ctrRight = 0;
		for(int i = 0;i<Q[b].size();i++)
		{
			int desti=Q[b][i];
			int dist=relDist(b,N,desti);
			if (dist<0){
				if(ctrLeft < C){
					maxFillLeft.push_back(i);
				}
			}
			else if (!(!(N%2) && dist == N/2)){
				if(ctrRight<C){
					maxFillRight.push_back(i);
				}
			}
			else
			{
				if(ctrLeft<C){
					maxFillLeft.push_back(i);
				}
				if(ctrRight<C){
					maxFillRight.push_back(i);
				}
			}
		}	


		short s = state;

		vector<unsigned int>& newbus = (s==-1?maxFillLeft:maxFillRight);
		for(int i = 0;i<Q[b].size() && newbus.size()+B.size()<C;i++)
		{
			if( (s==-1 && relDist(b,N,Q[b][i])>0 && (!(!(N%2) && relDist(b,N,Q[b][i]) == N/2))) || (s==1 && relDist(b,N,Q[b][i])<0) ){	
				newbus.push_back(i);
			}
		}
		
		return Response(newbus,s);	
	}
};


typedef AI_PENDEL AI_MY; // R.A.
