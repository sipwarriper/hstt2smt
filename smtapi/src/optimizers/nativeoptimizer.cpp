#include "nativeoptimizer.h"
#include "errors.h"
#include <iostream>

using namespace std;

NativeOptimizer::NativeOptimizer() : Optimizer(){

}

int NativeOptimizer::minimize(Encoder * e, int lb, int ub, bool useAssumptions, bool narrowBounds){

	if(!e->produceModels()){
		cerr << "The encoder must be enabled to produce models in order to use native optimization" << endl;
		exit(SOLVING_ERROR);
	}

	if(eventhandler != NULL)
			eventhandler->beforeNativeOptimizationCall(lb, ub);

	bool issat = e->optimize(lb,ub);

	if(eventhandler != NULL)
			eventhandler->afterNativeOptimizationCall(lb, ub,e);

	if(issat){
		int obj = e->getObjective();
		if(obj==INT_MIN){
			cerr << "The encoding must implement getObjective() to retrieve optimal solutions from native optimization" << endl;
		exit(SOLVING_ERROR);
		}
		if(eventhandler != NULL){
			eventhandler->onNewBoundsProved(obj,obj);
			eventhandler->onSATSolutionFound(lb,ub,obj);
			eventhandler->onProvedOptimum(obj);
		}
		return obj;
	}
	else{
		if(eventhandler != NULL){
			eventhandler->onUNSATBoundsDetermined(lb,ub);
			eventhandler->onProvedUNSAT();
		}
		return INT_MIN;
	}
}

int NativeOptimizer::maximize(Encoder * e, int lb, int ub, bool useAssumptions, bool narrowBounds){

	if(!e->produceModels()){
		cerr << "The encoder must be enabled to produce models in order to optimize" << endl;
		exit(SOLVING_ERROR);
	}

	if(eventhandler != NULL)
			eventhandler->beforeNativeOptimizationCall(lb, ub);

	bool issat = e->optimize(lb,ub);

	if(eventhandler != NULL)
			eventhandler->afterNativeOptimizationCall(lb, ub,e);

	if(issat){
		int obj = e->getObjective();
		if(obj==INT_MIN){
			cerr << "The encoding must implement getObjective() to retrieve optimal solutions from native optimization" << endl;
		exit(SOLVING_ERROR);
		}
		if(eventhandler != NULL){
			eventhandler->onNewBoundsProved(obj,obj);
			eventhandler->onSATSolutionFound(lb,ub,obj);
			eventhandler->onProvedOptimum(obj);
		}
		return obj;
	}
	else{
		if(eventhandler != NULL){
			eventhandler->onUNSATBoundsDetermined(lb,ub);
			eventhandler->onProvedUNSAT();
		}
		return INT_MIN;
	}
}

NativeOptimizer::~NativeOptimizer() {
}
