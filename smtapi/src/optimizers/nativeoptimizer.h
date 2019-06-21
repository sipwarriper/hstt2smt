#ifndef NATIVEOPTIMIZER_DEFINITION
#define NATIVEOPTIMIZER_DEFINITION


#include <vector>
#include <set>
#include <string>
#include <map>
#include <sstream>
#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include "encoder.h"
#include "encoding.h"
#include "optimizer.h"

using namespace std;

class NativeOptimizer : public Optimizer{

public:
	NativeOptimizer();

	~NativeOptimizer();

	int minimize(Encoder * e, int LB, int UB, bool useAssumptions=false, bool narrowBounds=false);

	int maximize(Encoder * e, int LB, int UB, bool useAssumptions=false, bool narrowBounds=false);

};

#endif

