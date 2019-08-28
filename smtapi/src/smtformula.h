#ifndef SMTFORMULA_DEFINITION
#define SMTFORMULA_DEFINITION

#include <vector>
#include <string>
#include <map>
#include <sstream>
#include <stdlib.h>
#include "mdd.h"
#include "smtapi.h"
#include "mddbuilder.h"

using namespace std;

namespace smtapi{

/*
 * This class is meant to manage the creation and assertion of
 * a CNF SMT formula.
 * It only handles the theory of LIA.
 * Variables are identified by a name and up to 3 subindices.
 * It also provides unnamed variables, which will usually be auxilliary variables
 * which are going to be used only once.
 * It is a pure virtual class which delegates to child classes
 * the assertion of formulas, either to solver APIs or to standard files.
 * It implements SAT encodings of many common constraints:
 * 	- at-most-one
 * 	- at-least-one
 * 	- exactly-one
 * 	- cardinality constraint
 * 	- pseudo-Boolean constraint
 * 	- at-most-one pseudo-Boolean constraint
 */

enum FORMULA_TYPE{
	SATFORMULA,
	MAXSATFORMULA,
	SMTFORMULA,
	OMTMINFORMULA,
	OMTMAXFORMULA
};

enum AMOEncoding {
	AMO_QUAD,
	AMO_LOG,
	AMO_LADDER,
	AMO_HEULE,
	AMO_COMMANDER, //Under developement, not working
};

enum CardinalityEncoding {
	CARD_SORTER,
	CARD_TOTALIZER
};

enum PBEncoding {
	PB_BDD,
	PB_BDD2,
	PB_SWC,
	PB_GT,
	PB_GT2,
	PB_GPW,
	PB_LPW,
	PB_GBM,
	PB_LBM
};

enum AMOPBEncoding {
	AMOPB_BDD,
	AMOPB_BDD2,
	AMOPB_SWC,
	AMOPB_GT,
	AMOPB_GT2,
	AMOPB_GPW,
	AMOPB_LPW,
	AMOPB_GBM,
	AMOPB_LBM,
	AMOPB_AMOMDD,
	AMOPB_AMOMDD2,
	AMOPB_IMPCHAIN,
	AMOPB_AMOBDD,
	AMOPB_GSWC,
	AMOPB_SORTER,
	AMOPB_GGT,
	AMOPB_GGT2,
	AMOPB_GGPW,
	AMOPB_GLPW,
	AMOPB_GGBM,
	AMOPB_GLBM
};

extern map<AMOPBEncoding,PBEncoding> amopb_pb_rel;
class SMTFormula {

private:

	/*
	 * Boolean/Int variables are identified in the ranges [1,nBoolVars] and
	 * [1,nIntVars] respectively. Any variable with an identifier out of this
	 * range will cause a panic exit in time of solver API/file formula assertion,
	 * with error code UNDEFINEDVARIABLE_ERROR.
	 */
	int nBoolVars; //Number of Boolean variables
	int nIntVars; //Number of Int variables

	int nClauses;//Number of clauses
	vector<clause> clauses; //Vector of clauses
	vector<clause> softclauses; //Vector of soft clauses. If non-empty, is a partial MaxSat problem
	vector<int> weights; //Vector of weights of the soft clauses.
	vector<intvar> softclausevars; //Vector of soft clauses.

	map<string,boolvar> mapBoolVars; //Map of Boolean variables identified by name
	map<string,intvar> mapIntVars; //Map of Int variables identified by name

	vector<string> boolVarNames; //Name of Boolvars indexed by id. Position 0 is "".
	vector<string> intVarNames; //Name of Intvars indexed by id. Position 0 is "".
	vector<bool> declareVar; //True iff if the i-th int var is a variable has to be declared (i.e. is not a soft clause var)

	boolvar falsevar; //Singleton trivially false variable
	boolvar truevar; //Single trivially true variable

	intsum objFunc; //Objective function in case it is OMT
	bool hasObjFunc; //True iff an objective function has been defined
	bool hassoftclauseswithvars; //True if some soft clause has an associated variable
	bool isMinimization; //True if is an OMT minimization problem
	int LB; //Lower bound for the objective function
	int UB; //Upper bound for the objective function


	void addOrderEncoding(int x, vector<literal> & lits);

	//Adds the codification of Sorter [x1,x2] -> [y1,y2]
	void addTwoComparator(const literal &x1, const literal &x2, literal &y1, literal &y2);

	//Adds the codification of "y is the result of merging x1,x2". Used in cardinality constraint
	void addSimplifiedMerge(const vector<literal> &x1, const vector<literal> &x2, vector<literal> &y, int c);

	void addQuadraticMerge(const vector<literal> &x1, const vector<literal> &x2, vector<literal> &y);

	void addTotalizer(const vector<literal> &x, vector<literal> &y);

	void addTotalizer(const vector<literal> &x, vector<literal> &y, int k);

	literal assertMDDLEQAbio(MDD * mdd);

	literal assertMDDLEQAbio(MDD * mdd, vector<literal> & asserted);

	literal assertMDDGTAbio(MDD * mdd);

	literal assertMDDGTAbio(MDD * mdd, vector<literal> & asserted, vector<literal> & elses);

	void addAMOPBSWC(const vector<vector<int> > & Q, const vector<vector<literal> > & X, int K);

	void addAMOPBSorter(const vector<vector<int> > & Q, const vector<vector<literal> > & X, int K);

	void addAMOPBGeneralizedTotalizer(const vector<vector<int> > & Q, const vector<vector<literal> > & X, int K);

	void addAMOPBGeneralizedTotalizer2(const vector<vector<int> > & Q, const vector<vector<literal> > & X, int K);

	literal addPolynomialWatchdog(const vector<vector<int> > & Q, const vector<vector<literal> > & X, int K, bool useSorter);

	void addAMOPBLocalPolynomialWatchdog(const vector<vector<int> > & Q, const vector<vector<literal> > & X, int K, bool useSorter);

	void addAMOPBGlobalPolynomialWatchdog(const vector<vector<int> > & Q, const vector<vector<literal> > & X, int K, bool useSorter);


	string ssubs(const string & var, int i1) const;
	string ssubs(const string & var, int i1, int i2) const;
	string ssubs(const string & var, int i1, int i2, int i3) const;


public:
	//Default constructor
	SMTFormula();

	//Destructor
	~SMTFormula();

	//Return the type of the formula based on its content
	FORMULA_TYPE getType() const;

	bool hasSoftClausesWithVars() const;

	int getNBoolVars() const;

	int getNIntVars() const;

	int getNClauses() const;

	int getNSoftClauses() const;

	const vector<clause> & getClauses() const;

	const vector<clause> & getSoftClauses() const;

	const vector<int> & getWeights() const;

	const vector<intvar> & getSoftClauseVars() const;

	int getHardWeight() const;

	const vector<string> & getBoolVarNames() const;

	const vector<string> & getIntVarNames() const;

	bool isDeclareVar(int id) const;

	const intsum & getObjFunc() const;

	//Get the trivially false variable
	boolvar falseVar();

	//Get the trivially true variable
	boolvar trueVar();


	//The name of a variable cannot contain the character "_"


	//Get a new unnamed Boolean variable
	boolvar newBoolVar();
	//Get a new named Boolean variable, with up to 3 subindices in the name
	boolvar newBoolVar(const string & var);
	boolvar newBoolVar(const string & var, int i1);
	boolvar newBoolVar(const string & var, int i1, int i2);
	boolvar newBoolVar(const string & var, int i1, int i2, int i3);


	//Get a new unnamed Int variable
	intvar newIntVar(bool declare=true);
	//Get a new named Int variable, with up to 3 subindices in the name
	intvar newIntVar(const string & var, bool declare=true);
	intvar newIntVar(const string & var, int i1, bool declare=true);
	intvar newIntVar(const string & var, int i1, int i2, bool declare=true);
	intvar newIntVar(const string & var, int i1, int i2, int i3, bool declare=true);

	//Get named Boolean variable by name and subindices
	boolvar bvar(const string & var) const;
	boolvar bvar(const string & var, int i1) const;
	boolvar bvar(const string & var, int i1, int i2) const;
	boolvar bvar(const string & var, int i1, int i2, int i3) const;

	//Get named Int variable by name and subindices
	intvar ivar(const string & var) const;
	intvar ivar(const string & var, int i1) const;
	intvar ivar(const string & var, int i1, int i2) const;
	intvar ivar(const string & var, int i1, int i2, int i3) const;

	void minimize(const intsum & sum);
	void maximize(const intsum & sum);
	void setLowerBound(int LB);
	void setUpperBound(int UB);
	int getLowerBound();
	int getUpperBound();

	static int getIValue(const intvar & var, const vector<int> & vals);
	static bool getBValue(const boolvar & var, const vector<bool> & vals);

	//Add the empty clause to the formula
	void addEmptyClause();

	//Add clause 'c' to the formula
	void addClause(const clause &c);

	//Add soft clause 'c' to the formula
	void addSoftClause(const clause &c, int weight = 1);

	//Add soft clause 'c' to the formula
	void addSoftClauseWithVar(const clause &c, int weight, const intvar & var);

	//All all the clauses in 'v' to the formula
	void addClauses(const vector<clause> &c);

	//Adds at-least-one constraint on the literals in 'v'
	void addALO(const vector<literal> & v);

	//Adds at-most-one constraint on the literals in 'v'
	void addAMO(const vector<literal> & v, AMOEncoding enc = AMO_QUAD);

	//Adds exactly-one constraint on the literals in 'v'
	void addEO(const vector<literal> & v, AMOEncoding enc = AMO_QUAD);

    //Adds at-least-K constraint on the literals in 'v'
    void addALK(const vector<literal> & v, int K);

    //Adds at-least-K constraint on the literals in 'v'... forces c to be true if the alk is violated (!alk_enc -> c)
    void addALKWithCheckVar(const vector<literal> & v, int K, boolvar c);

    //Adds at-most-K constraint on the literals in 'v'
	void addAMK(const vector<literal> & v, int K, CardinalityEncoding enc = CARD_SORTER);

    //Adds at-most-K constraint on the literals in 'v'.... forces c to be true if the amk is violated (!amk_enc-> c)
    void addAMKWithCheckVar(const vector<literal> & v, int K, boolvar c, CardinalityEncoding enc = CARD_SORTER);

    //Adds exactly-K constraint on the literals in 'v'
    void addEK(const vector<literal> & v, int K);

    //Adds exactly-K constraint on the literals in 'v'... forces c to be true if the ek is violated (!ek_enc-> c)
    void addEKWithCheckVar(const vector<literal> & v, int K, boolvar c);

	//Adds PB constraint Q*X <= K
	void addPB(const vector<int> & Q, const vector<literal> & X, int K, PBEncoding = PB_BDD);

	//Adds PB constraint Q*X >= K
	void addPBGEQ(const vector<int> & Q, const vector<literal> & X, int K, PBEncoding = PB_BDD);

	//Adds AMO-PB constraint Q*X <= K
	void addAMOPB(const vector<vector<int> > & Q, const vector<vector<literal> > & X, int K, AMOPBEncoding encoding = AMOPB_AMOMDD);

	//Adds AMO-PB constraint Q*X >= K
	void addAMOPBGEQ(const vector<vector<int> > & Q, const vector<vector<literal> > & X, int K, AMOPBEncoding encoding = AMOPB_AMOMDD);

	//Adds PB constraint Q*X <= K, and returns a PersistentAMOPB that can be incrementally constrained
	//PersistentAMOPB * addPersistentPB(const vector<int> & Q, const vector<literal> & X, int K, PBEncoding = PB_BDD);

	//Adds AMO-PB constraint Q*X <= K, and returns a PersistentAMOPB that can be incrementally constrained
	/*MDDBuilder * addPersistentAMOPB(const vector<vector<int> > & Q, const vector<vector<literal> > & X, int K, PBEncoding encoding = AMOPB_AMOMDD);

	MDDBuilder * addPersistentAMOPBGT(const vector<vector<int> > & Q, const vector<vector<literal> > & X, int K, PBEncoding encoding = AMOPB_AMOMDD);

	//Adds AMO-PB constraint Q*X >= K
	void addAMOPBGEQ(const vector<vector<int> > & Q, const vector<vector<literal> > & X, int K, PBEncoding encoding = AMOPB_AMOMDD);*/

	/*void addUB(int ub, MDDBuilder * mb);

	void addLB(int lb, MDDBuilder * mb);
*/

		//Adds the codification of "y is the x list sorted  decreasingly". Used in cardinality constraint
	void addSorting(const vector<literal> &x, vector<literal> &y);


	//Adds the codification of "y is the result of merging x1,x2". Used in cardinality constraint
	void addMerge(const vector<literal> &x1, const vector<literal> &x2, vector<literal> &y);

    void copy_to(SMTFormula *f2) const;
};

}

#endif

