#include "basiccontroller.h"
#include <limits.h>
#include "dimacsfileencoder.h"
#include "smtlib2fileencoder.h"
#include "basiceventhandler.h"
#include "errors.h"

#ifdef USEYICES
#include "yices2apiencoder.h"
#endif


BasicController::BasicController(SolvingArguments * sargs, Encoding * enc, bool minimize, int lb, int ub, EventHandler * eh){

	this->sargs = sargs;
	this->encoding = enc;
	this->minimize = minimize;
	this->LB = sargs->getIntOption(LOWER_BOUND);
	this->UB = sargs->getIntOption(UPPER_BOUND);
	if(this->LB==INT_MIN)
		this->LB = lb;
	if(this->UB==INT_MIN)
		this->UB = ub;

	this->eh = eh;
}

BasicController::~BasicController() {

}

void BasicController::run() {
	if(sargs->getBoolOption(OUTPUT_ENCODING)){
		FileEncoder * e = sargs->getFileEncoder(encoding);
		SMTFormula * f = encoding->encode(LB,UB);
		e->createFile(cout,f);
		delete e;
		delete f;
	}
	else{
		Optimizer * opti = sargs->getOptimizer();
		Encoder * e = sargs->getEncoder(encoding);
		if(eh == NULL)
			eh =  new BasicEventHandler(encoding,sargs);
		opti->setEventHandler(eh);
		if(minimize)
			opti->minimize(e, LB,UB,sargs->getBoolOption(USE_ASSUMPTIONS),sargs->getBoolOption(NARROW_BOUNDS));
		else
			opti->maximize(e, LB, UB,sargs->getBoolOption(USE_ASSUMPTIONS),sargs->getBoolOption(NARROW_BOUNDS));
		delete opti;
		delete e;
	}
}

