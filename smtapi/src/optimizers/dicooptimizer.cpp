#include "dicooptimizer.h"
#include "errors.h"
#include <iostream>

using namespace std;

DicoOptimizer::DicoOptimizer() : Optimizer(){

}

int DicoOptimizer::minimize(Encoder * e, int lb, int ub, bool useAssumptions, bool narrowBounds) {

	if(!e->produceModels()){
		cerr << "The encoder must be enabled to produce models in order to optimize" << endl;
		exit(SOLVING_ERROR);
	}

	bool satcheck = true;
	bool issat = false;
	bool satverified = false;
	int firstub = ub;
	int obj_val;
	int lastval=ub;

	int checkbound;

	if(useAssumptions)
		e->initAssumptionOptimization(lb,ub);

	while(ub > lb | !satverified){
		checkbound = (ub + lb)/2;
		if(eventhandler != NULL)
			eventhandler->beforeSatisfiabilityCall(lb, checkbound);

		satcheck = useAssumptions ?
					e->checkSATAssuming(lb,checkbound):
					e->checkSAT(lb,checkbound);

		if(eventhandler != NULL)
			eventhandler->afterSatisfiabilityCall(lb, checkbound,e);

		if(!issat)
			issat = satcheck;

		if(satcheck){
			satverified = true;
			if(narrowBounds && useAssumptions)
				e->narrowBounds(lb,checkbound);

			if(e->produceModels() && e->getObjective()!=INT_MIN){
				obj_val = e->getObjective();
				if(eventhandler != NULL){
					eventhandler->onNewBoundsProved(lb,obj_val);
					eventhandler->onSATSolutionFound(lb,checkbound,obj_val);
				}
			}
			else{
				obj_val = checkbound;
				if(eventhandler != NULL)
					eventhandler->onNewBoundsProved(lb,obj_val);
			}

			lastval = obj_val;
			ub = obj_val;
		}
		else{
			if(lb == firstub)
				satverified = true;

			if(narrowBounds && useAssumptions)
				e->narrowBounds(checkbound+1,ub);

			if(eventhandler != NULL){
				eventhandler->onNewBoundsProved(checkbound+1,ub);
				eventhandler->onUNSATBoundsDetermined(lb,checkbound);
			}
			lb = checkbound+1;
		}
	}

	if(issat){
		if(eventhandler != NULL)
			eventhandler->onProvedOptimum(lastval);
		return lastval;
	}
	else{
		if(eventhandler != NULL)
			eventhandler->onProvedUNSAT();
		return INT_MIN;
	}
}


int DicoOptimizer::maximize(Encoder * e, int lb, int ub, bool useAssumptions, bool narrowBounds) {

	if(!e->produceModels()){
		cerr << "The encoder must be enabled to produce models in order to optimize" << endl;
		exit(SOLVING_ERROR);
	}

	bool satcheck = true;
	bool issat = false;
	bool satverified = false;
	int firstlb = lb;
	int obj_val;
	int lastval=lb;

	int checkbound;

	if(useAssumptions)
		e->initAssumptionOptimization(lb,ub);

	while(ub > lb | !satverified){
		checkbound = (ub + lb + 1)/2;
		if(eventhandler != NULL)
			eventhandler->beforeSatisfiabilityCall(checkbound,ub);

		satcheck = useAssumptions ?
					e->checkSATAssuming(checkbound,ub):
					e->checkSAT(checkbound,ub);

		if(eventhandler != NULL)
			eventhandler->afterSatisfiabilityCall(checkbound,ub,e);

		if(!issat)
			issat = satcheck;

		if(satcheck){
			satverified = true;
			if(narrowBounds && useAssumptions)
				e->narrowBounds(checkbound,ub);

			if(e->produceModels() && e->getObjective()!=INT_MIN){
				obj_val = e->getObjective();
				if(eventhandler != NULL){
					eventhandler->onNewBoundsProved(obj_val,ub);
					eventhandler->onSATSolutionFound(checkbound,ub,obj_val);
				}
			}
			else{
				obj_val = checkbound;
				if(eventhandler != NULL)
					eventhandler->onNewBoundsProved(obj_val,ub);
			}
			lastval = obj_val;
			lb = obj_val;
		}
		else{
			if(ub == firstlb)
				satverified = true;

			if(narrowBounds && useAssumptions)
				e->narrowBounds(lb,checkbound-1);

			if(eventhandler != NULL){
				eventhandler->onNewBoundsProved(lb,checkbound-1);
				eventhandler->onUNSATBoundsDetermined(checkbound,ub);
			}
			ub = checkbound-1;
		}
	}

	if(issat){
		if(eventhandler != NULL)
			eventhandler->onProvedOptimum(lastval);
		return lastval;
	}
	else{
		if(eventhandler != NULL)
			eventhandler->onProvedUNSAT();
		return INT_MIN;
	}
}



DicoOptimizer::~DicoOptimizer() {

}
