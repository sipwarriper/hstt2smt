#ifndef GLUCOSEAPIENCODER_DEFINITION
#define GLUCOSEAPIENCODER_DEFINITION

#include "apiencoder.h"
#include "SimpSolver.h"
#include "SolverTypes.h"
#include "Vec.h"

using namespace std;
using namespace smtapi;
using namespace Glucose;


/*
 * This class asserts an SMT formula to the Yices2 API.
 */
class GlucoseAPIEncoder : public APIEncoder {

private:

	SimpSolver * s;

	vector<Var> vars;

	int lastVar;
	int lastClause;

	Lit getLiteral(const literal & l, const vector<Var> & boolvars);
	bool assertAndCheck(int lb, int ub, vector<literal> * assumptions);

public:
	//Default constructor
	GlucoseAPIEncoder(Encoding * enc);

	//Destructor
	~GlucoseAPIEncoder();

	bool checkSAT(int lb, int ub);
	bool checkSATAssuming(int lb, int ub);
	void narrowBounds(int lb, int ub);

};

#endif

