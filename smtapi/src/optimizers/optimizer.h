#ifndef OPTIMIZER_DEFINITION
#define OPTIMIZER_DEFINITION

#include "encoder.h"
#include "eventhandler.h"


class Optimizer{

protected :

	EventHandler * eventhandler;

public:

	Optimizer();

	virtual ~Optimizer();

	virtual bool checkSAT(Encoder * e, int LB, int UB);

	virtual int minimize(Encoder * e, int LB, int UB, bool useAssumptions=false, bool narrowBounds=false);

	virtual int maximize(Encoder * e, int LB, int UB, bool useAssumptions=false, bool narrowBounds=false);

	void setEventHandler(EventHandler * eh);

// 	bool printoptsolution;
// 	bool printnonoptsolutions;
// 	bool printsteps;
// 	bool printstats;

};

#endif

