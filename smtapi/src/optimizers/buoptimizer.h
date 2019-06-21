#ifndef BUOPTIMIZER_DEFINITION
#define BUOPTIMIZER_DEFINITION


#include "encoder.h"
#include "optimizer.h"

using namespace std;

class BUOptimizer : public Optimizer{

public:

	BUOptimizer();

	~BUOptimizer();

	int maximize(Encoder * e, int LB, int UB, bool useAssumptions=false, bool narrowBounds=false);

	int minimize(Encoder * e, int LB, int UB, bool useAssumptions=false, bool narrowBounds=false);

};

#endif

// template<typename T, typename = std::enable_if<std::is_base_of<MyClass, T>::value>>
// template<typename T, typename std::enable_if<std::is_base_of<MyClass, T>::value>
