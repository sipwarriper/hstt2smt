#include "buoptimizer.h"
#include "errors.h"
#include <iostream>

using namespace std;

BUOptimizer::BUOptimizer() : Optimizer(){

}

int BUOptimizer::maximize(Encoder * e, int lb, int ub, bool useAssumptions, bool narrowBounds) {

	if(!e->produceModels()){
		cerr << "The encoder must be enabled to produce models in order to optimize" << endl;
		exit(SOLVING_ERROR);
	}

	bool satcheck = true;
	bool issat = false;
	int obj_val;
	int lastlb=lb;

	while(satcheck && ub >= lb){

		if(eventhandler != NULL)
			eventhandler->beforeSatisfiabilityCall(lb, ub);

		satcheck = e->checkSAT(lb,ub);

		if(eventhandler != NULL)
			eventhandler->afterSatisfiabilityCall(lb, ub,e);

		if(!issat)
			issat = satcheck;

		if(satcheck){
			if(e->produceModels() && e->getObjective()!=INT_MIN){
				obj_val = e->getObjective();
				if(eventhandler != NULL){
					eventhandler->onNewBoundsProved(obj_val,ub);
					eventhandler->onSATSolutionFound(lb,ub,obj_val);
				}
			}
			else{
				obj_val = lb;
				if(eventhandler != NULL)
					eventhandler->onNewBoundsProved(obj_val,ub);
			}

			lastlb = obj_val;
			lb=obj_val+1;
		}
		else{
			if(eventhandler != NULL){
				eventhandler->onNewBoundsProved(lastlb,lb-1);
				eventhandler->onUNSATBoundsDetermined(lb,ub);
			}
		}
	}

	if(issat){
		if(eventhandler != NULL)
			eventhandler->onProvedOptimum(lastlb);
		return lb-1;
	}
	else{
		if(eventhandler != NULL)
			eventhandler->onProvedUNSAT();
		return INT_MIN;
	}
}

int BUOptimizer::minimize(Encoder * e, int lb, int ub, bool useAssumptions, bool narrowBounds)
{
	bool satcheck = false;
	int obj_val;
	int checkub=lb;

	if(useAssumptions)
		e->initAssumptionOptimization(lb,ub);

	while(!satcheck && checkub <= ub){
		if(eventhandler != NULL)
			eventhandler->beforeSatisfiabilityCall(lb, checkub);

			satcheck = useAssumptions ?
					e->checkSATAssuming(lb,checkub):
					e->checkSAT(lb,checkub);

		if(eventhandler != NULL)
			eventhandler->afterSatisfiabilityCall(lb, checkub,e);

		if(satcheck){
			if(eventhandler != NULL){
				eventhandler->onNewBoundsProved(lb,checkub);
				eventhandler->onSATSolutionFound(lb,checkub,checkub);
			}
		}
		else{
			lb = checkub;
			if(narrowBounds && useAssumptions)
				e->narrowBounds(lb+1,ub);
			if(eventhandler != NULL){
				eventhandler->onNewBoundsProved(lb,ub);
				eventhandler->onUNSATBoundsDetermined(lb,checkub);
			}
			checkub++;
		}
	}
	if(satcheck){
		if(eventhandler != NULL)
			eventhandler->onProvedOptimum(satcheck);
		return checkub;
	}
	else{
		if(eventhandler != NULL)
			eventhandler->onProvedUNSAT();
		return INT_MIN;
	}
}



BUOptimizer::~BUOptimizer() {

}
