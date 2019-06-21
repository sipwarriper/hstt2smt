#include "basiceventhandler.h"
#include "errors.h"

BasicEventHandler::BasicEventHandler(Encoding * encoding, SolvingArguments * sargs) : EventHandler(){
	this->encoding = encoding;
	this->sargs = sargs;
}

BasicEventHandler::~BasicEventHandler(){

}

void BasicEventHandler::afterSatisfiabilityCall(int lb, int ub, Encoder * encoder){
	if(sargs->getBoolOption(PRINT_CHECKS_STATISTICS)){
		cout << "c stats ";

		//Bounds and time
		cout << lb << ";";
		cout << ub << ";";
		cout << encoder->getCheckTime() << ";";
		cout << encoder->getSolverCheckTime() << ";";

		//Formula sizes
		cout << encoder->getNBoolVars() << ";";
		cout << encoder->getNClauses() << ";";

		//Solving statistics
		cout << encoder->getNRestarts() << ";";
		cout << encoder->getNSimplify() << ";";
		cout << encoder->getNReduce() << ";";
		cout << encoder->getNDecisions() << ";";
		cout << encoder->getNPropagations() << ";";
		cout << encoder->getNConflicts() << ";";
		cout << encoder->getNTheoryPropagations() << ";";
		cout << encoder->getNTheoryConflicts() << ";";

		cout << endl;
	}
}

void BasicEventHandler::afterNativeOptimizationCall(int lb, int ub, Encoder * encoder){
	afterSatisfiabilityCall(lb,ub,encoder);
}

void BasicEventHandler::onNewBoundsProved(int lb, int ub){
	if(sargs->getBoolOption(PRINT_CHECKS))
		cout << "c lb/ub " << lb << " " << ub << endl;
}

void BasicEventHandler::onSATSolutionFound(int & lb, int & ub, int & obj_val){
	if(sargs->getBoolOption(PRODUCE_MODELS) && sargs->getBoolOption(PRINT_NOOPTIMAL_SOLUTIONS)){
		cout << "v ";
		if(!encoding->printSolution(cout))
			cout << " [Solution printing not implemented]";
		cout << endl;
	}
}


void BasicEventHandler::onProvedOptimum(int opt){
	if(sargs->getBoolOption(PRODUCE_MODELS) && sargs->getBoolOption(PRINT_NOOPTIMAL_SOLUTIONS) && !sargs->getBoolOption(PRINT_OPTIMAL_SOLUTION)){
		cout << "v ";
		if(!encoding->printSolution(cout))
			cout << " [Solution printing not implemented]";
		cout << endl;
	}
	cout << "s OPTIMUM FOUND" << endl;
	cout << "o " << encoding->getObjective() << endl;
}

void BasicEventHandler::onProvedSAT(){
	cout << "s SATISFIABLE" << endl;
}

void BasicEventHandler::onProvedUNSAT(){
	cout << "s UNSATISFIABLE" << endl;
}


