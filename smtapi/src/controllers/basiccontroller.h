#ifndef BASICCONTROLLER_DEFINITION
#define BASICCONTROLLER_DEFINITION


#include <vector>
#include <set>
#include <string>
#include <map>
#include <sstream>
#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include "encoder.h"
#include "fileencoder.h"
#include "smtformula.h"
#include "solvingarguments.h"


class BasicController {
 protected:

	int LB;
	int UB;
	Encoding * encoding;
    bool minimize;
    EventHandler * eh;
    SolvingArguments * sargs;


public:
	BasicController(SolvingArguments * sargs, Encoding * enc, bool minimize, int lb, int ub, EventHandler * eh = NULL);
	virtual ~BasicController();

	virtual void run();
};

#endif

