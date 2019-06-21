#include "uboptimizer.h"
#include "errors.h"
#include <iostream>

using namespace std;

UBOptimizer::UBOptimizer() : Optimizer(){

}

int UBOptimizer::maximize(Encoder * e, int lb, int ub, bool useAssumptions, bool narrowBounds)
{
	if(!e->produceModels()){
		cerr << "The encoder must be enabled to produce models in order to optimize" << endl;
		exit(SOLVING_ERROR);
	}
	bool satcheck = false;
	int obj_val;
	int checklb=ub;

	if(useAssumptions)
		e->initAssumptionOptimization(lb,ub);

	while(!satcheck && checklb >= lb){
		if(eventhandler != NULL)
			eventhandler->beforeSatisfiabilityCall(checklb, ub);

		satcheck = useAssumptions ?
					e->checkSATAssuming(checklb,ub):
					e->checkSAT(checklb,ub);

		if(eventhandler != NULL)
			eventhandler->afterSatisfiabilityCall(checklb, ub,e);

		if(satcheck){
			if(eventhandler != NULL){
				eventhandler->onNewBoundsProved(checklb,ub);
				eventhandler->onSATSolutionFound(checklb,ub,checklb);
			}
		}
		else{
			ub = checklb;
			if(narrowBounds && useAssumptions)
				e->narrowBounds(lb,ub-1);
			if(eventhandler != NULL){
				eventhandler->onNewBoundsProved(lb,ub);
				eventhandler->onUNSATBoundsDetermined(checklb,ub);
			}
			checklb--;
		}
	}
	if(satcheck){
		if(eventhandler != NULL)
			eventhandler->onProvedOptimum(satcheck);
		return checklb;
	}
	else{
		if(eventhandler != NULL)
			eventhandler->onProvedUNSAT();
		return INT_MIN;
	}
}


int UBOptimizer::minimize(Encoder * e, int lb, int ub, bool useAssumptions, bool narrowBounds)
{

	if(!e->produceModels()){
		cerr << "The encoder must be enabled to produce models in order to optimize" << endl;
		exit(SOLVING_ERROR);
	}

	bool satcheck = true;
	bool issat = false;
	int obj_val;
	int lastub=ub;



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
					eventhandler->onNewBoundsProved(lb,obj_val);
					eventhandler->onSATSolutionFound(lb,ub,obj_val);
				}
			}
			else{
				obj_val = ub;
				if(eventhandler != NULL)
					eventhandler->onNewBoundsProved(lb,obj_val);
			}

			lastub = obj_val;
			ub=obj_val-1;
		}
		else{
			if(eventhandler != NULL){
				eventhandler->onNewBoundsProved(ub+1,lastub);
				eventhandler->onUNSATBoundsDetermined(lb,ub);
			}
		}
	}

	if(issat){
		if(eventhandler != NULL)
			eventhandler->onProvedOptimum(lastub);
		return ub+1;
	}
	else{
		if(eventhandler != NULL)
			eventhandler->onProvedUNSAT();
		return INT_MIN;
	}
}

UBOptimizer::~UBOptimizer() {

}
