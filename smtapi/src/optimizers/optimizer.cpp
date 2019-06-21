#include "optimizer.h"
#include "errors.h"
#include <iostream>
#include "limits.h"
using namespace std;

Optimizer::Optimizer() {
	eventhandler=NULL;
}

Optimizer::~Optimizer() {

}


bool Optimizer::checkSAT(Encoder * e, int lb, int ub){

	if(eventhandler != NULL)
		eventhandler->beforeSatisfiabilityCall(lb, ub);

	bool satcheck = e->checkSAT(lb,ub);

	if(eventhandler != NULL)
		eventhandler->afterSatisfiabilityCall(lb, ub,e);


	if(satcheck){
		int obj;
		if(eventhandler != NULL){
			eventhandler->onSATSolutionFound(lb,ub,obj);
			eventhandler->onProvedSAT();
		}
	}
	else
		if(eventhandler != NULL)
			eventhandler->onProvedUNSAT();

	return satcheck;
}

int Optimizer::minimize(Encoder * e, int LB, int UB, bool useAssumptions, bool narrowBounds){
	cerr << "Bad configuration: unsupported functionality" << endl;
	exit(UNSUPPORTEDFUNC_ERROR);
}

int Optimizer::maximize(Encoder * e, int LB, int UB, bool useAssumptions, bool narrowBounds){
	cerr << "Bad configuration: unsupported functionality" << endl;
	exit(UNSUPPORTEDFUNC_ERROR);
}

void Optimizer::setEventHandler(EventHandler * eh){
	eventhandler = eh;
}
