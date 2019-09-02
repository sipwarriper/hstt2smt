#include "smtformula.h"
#include "amopbmddbuilder.h"
#include "amopbbddbuilder.h"
#include "mdd.h"
#include "errors.h"
#include "util.h"
#include <limits.h>
#include <algorithm>
#include <math.h>


namespace smtapi{


std::map<AMOPBEncoding,PBEncoding> amopb_pb_rel = {
	{AMOPB_BDD,PB_BDD},
	{AMOPB_SWC,PB_SWC},
	{AMOPB_GT,PB_GT},
	{AMOPB_GPW,PB_GPW},
	{AMOPB_LPW,PB_LPW},
	{AMOPB_GBM,PB_GBM},
	{AMOPB_LBM,PB_LBM}
};

std::string SMTFormula::defaultauxboolvarpref = "b";
std::string SMTFormula::defaultauxintvarpref = "i";

SMTFormula::SMTFormula() {
	nBoolVars=0;
	nIntVars=0;
	nClauses=0;

	boolVarNames.push_back("");
	intVarNames.push_back("");
	declareVar.push_back(false);

	hasObjFunc = false;
	isMinimization = false;
	LB=INT_MIN;
	UB=INT_MAX;
	hassoftclauseswithvars = false;
    
    use_predef_lits = false;
    use_predef_order = false;
    
    auxboolvarpref=defaultauxboolvarpref;
    auxintvarpref=defaultauxintvarpref;
}

SMTFormula::~SMTFormula() {

}

FORMULA_TYPE SMTFormula::getType() const{
	if(nIntVars > 0){
		if(hasObjFunc){
			if(isMinimization)
				return OMTMINFORMULA;
			else
				return OMTMAXFORMULA;
		}
		else
			return SMTFORMULA;
	}
	else if(!softclauses.empty())
		return MAXSATFORMULA;
	else return SATFORMULA;
}
    
    void SMTFormula::setAuxBoolvarPref(const std::string & s){
    auxboolvarpref=s;
}
    
    void SMTFormula::setAuxIntvarPref(const std::string & s){
    auxintvarpref=s;
}
    
void SMTFormula::setDefaultAuxBoolvarPref(){
    auxboolvarpref=defaultauxboolvarpref;
}

void SMTFormula::setDefaultIntvarPref(){
    auxintvarpref=defaultauxintvarpref;
}
    
bool SMTFormula::usePredefDecs() const{
    return use_predef_lits;
}
    
bool SMTFormula::usePredefOrder() const{
    return use_predef_order;
}
    
void SMTFormula::setUsePredefDecs(const std::vector<literal> & lits, bool order){
    predef_lits = lits;
    use_predef_lits=true;
    use_predef_order=order;
}

void SMTFormula::getPredefDecs(std::vector<literal> & lits) const{
    lits = predef_lits;
}
    
bool SMTFormula::hasSoftClausesWithVars() const{
	return hassoftclauseswithvars;
}

int SMTFormula::getNBoolVars() const{
	return nBoolVars;
}

int SMTFormula::getNIntVars() const{
	return nIntVars;
}

int SMTFormula::getNClauses() const{
	return clauses.size();
}

int SMTFormula::getNSoftClauses() const{
	return softclauses.size();
}

const std::vector<clause> & SMTFormula::getClauses() const{
	return clauses;
}

const std::vector<clause> & SMTFormula::getSoftClauses() const{
	return softclauses;
}

const std::vector<int> & SMTFormula::getWeights() const{
	return weights;
}

const std::vector<intvar> & SMTFormula::getSoftClauseVars() const{
	return softclausevars;
}

int SMTFormula::getHardWeight() const{
	int sum = 2;
	for(int i : weights)
		sum+= i;
	return sum;
}

const std::vector<std::string> & SMTFormula::getBoolVarNames() const{
	return boolVarNames;
}

const std::vector<std::string> & SMTFormula::getIntVarNames() const{
	return intVarNames;
}

bool SMTFormula::isDeclareVar(int id) const{
	return declareVar[id];
}

const intsum &  SMTFormula::getObjFunc() const{
	return objFunc;
}

boolvar SMTFormula::trueVar(){
	if(truevar.id==0){
		truevar=newBoolVar();
		addClause(truevar);
	}
	return truevar;
}

boolvar SMTFormula::falseVar(){
	if(falsevar.id==0){
		falsevar=newBoolVar();
		addClause(!falsevar);
	}
	return falsevar;
}

boolvar SMTFormula::newBoolVar(){
	boolvar x;
	x.id=++nBoolVars;
	boolVarNames.push_back(auxboolvarpref+"__"+std::to_string(x.id));
	return x;
}

boolvar SMTFormula::newBoolVar(const std::string & s){
	boolvar x;
	x.id=++nBoolVars;
	mapBoolVars[s] = x;
	boolVarNames.push_back(s);
	return x;
}

boolvar SMTFormula::newBoolVar(const std::string & s, int i1){
	boolvar x;
	x.id=++nBoolVars;
	std::string s2 = ssubs(s,i1);
	mapBoolVars[s2] = x;
	boolVarNames.push_back(s2);
	return x;
}

boolvar SMTFormula::newBoolVar(const std::string & s, int i1, int i2){
	boolvar x;
	x.id=++nBoolVars;
	std::string s2 = ssubs(s,i1,i2);
	mapBoolVars[s2] = x;
	boolVarNames.push_back(s2);
	return x;
}

boolvar SMTFormula::newBoolVar(const std::string & s, int i1, int i2, int i3){
	boolvar x;
	x.id=++nBoolVars;
	std::string s2 = ssubs(s,i1,i2,i3);
	mapBoolVars[s2] = x;
	boolVarNames.push_back(s2);
	return x;
}

void SMTFormula::aliasBoolVar(const boolvar & x, const std::string & s){
	mapBoolVars[s] = x;
}

void SMTFormula::aliasBoolVar(const boolvar & x, const std::string & s, int i1){
	std::string s2 = ssubs(s,i1);
	mapBoolVars[s2] = x;
}

void SMTFormula::aliasBoolVar(const boolvar & x, const std::string & s, int i1, int i2){
	std::string s2 = ssubs(s,i1,i2);
	mapBoolVars[s2] = x;
}

void SMTFormula::aliasBoolVar(const boolvar & x, const std::string & s, int i1, int i2, int i3){
	std::string s2 = ssubs(s,i1,i2,i3);
	mapBoolVars[s2] = x;
}

intvar SMTFormula::newIntVar(bool declare){
	intvar x;
	x.id=++nIntVars;
	intVarNames.push_back(auxintvarpref+"__"+std::to_string(x.id));
	declareVar.push_back(declare);
	return x;
}

intvar SMTFormula::newIntVar(const std::string & s, bool declare){
	intvar x;
	x.id=++nIntVars;
	mapIntVars[s] = x;
	intVarNames.push_back(s);
	declareVar.push_back(declare);
	return x;
}

intvar SMTFormula::newIntVar(const std::string & s, int i1, bool declare){
	intvar x;
	x.id=++nIntVars;
	std::string s2 = ssubs(s,i1);
	mapIntVars[s2] = x;
	intVarNames.push_back(s2);
	declareVar.push_back(declare);
	return x;
}

intvar SMTFormula::newIntVar(const std::string & s, int i1, int i2, bool declare){
	intvar x;
	x.id=++nIntVars;
	std::string s2 = ssubs(s,i1,i2);
	mapIntVars[s2] = x;
	intVarNames.push_back(s2);
	declareVar.push_back(declare);
	return x;
}

intvar SMTFormula::newIntVar(const std::string & s, int i1, int i2, int i3, bool declare){
	intvar x;
	x.id=++nIntVars;
	std::string s2 = ssubs(s,i1,i2,i3);
	mapIntVars[s2] = x;
	intVarNames.push_back(s2);
	declareVar.push_back(declare);
	return x;
}



void SMTFormula::aliasIntVar(const intvar & x, const std::string & s){
	mapIntVars[s] = x;
}

void SMTFormula::aliasIntVar(const intvar & x, const std::string & s, int i1){
	std::string s2 = ssubs(s,i1);
	mapIntVars[s2] = x;
}

void SMTFormula::aliasIntVar(const intvar & x, const std::string & s, int i1, int i2){
	std::string s2 = ssubs(s,i1,i2);
	mapIntVars[s2] = x;
}

void SMTFormula::aliasIntVar(const intvar & x, const std::string & s, int i1, int i2, int i3){
	std::string s2 = ssubs(s,i1,i2,i3);
	mapIntVars[s2] = x;
}

boolvar SMTFormula::bvar(const std::string & s) const{
	return mapBoolVars.find(s)->second;
}

boolvar SMTFormula::bvar(const std::string & s, int i1) const{
	return mapBoolVars.find(ssubs(s,i1))->second;
}

boolvar SMTFormula::bvar(const std::string & s, int i1, int i2) const{
	return mapBoolVars.find(ssubs(s,i1,i2))->second;
}

boolvar SMTFormula::bvar(const std::string & s, int i1, int i2, int i3) const{
	return mapBoolVars.find(ssubs(s,i1,i2,i3))->second;
}

intvar SMTFormula::ivar(const std::string & s) const{
	return mapIntVars.find(s)->second;
}

intvar SMTFormula::ivar(const std::string & s, int i1) const{
	return mapIntVars.find(ssubs(s,i1))->second;
}

intvar SMTFormula::ivar(const std::string & s, int i1, int i2) const{
	return mapIntVars.find(ssubs(s,i1,i2))->second;
}

intvar SMTFormula::ivar(const std::string & s, int i1, int i2, int i3) const{
	return mapIntVars.find(ssubs(s,i1,i2,i3))->second;
}


std::string SMTFormula::ssubs(const std::string & s, int i1) const{
	char aux[50];
	sprintf(aux,"%s_%d",s.c_str(),i1);
	return aux;
}

std::string SMTFormula::ssubs(const std::string & s, int i1, int i2) const{
	char aux[50];
	sprintf(aux,"%s_%d_%d",s.c_str(),i1,i2);
	return aux;
}

std::string SMTFormula::ssubs(const std::string & s, int i1, int i2, int i3) const{
	char aux[50];
	sprintf(aux,"%s_%d_%d_%d",s.c_str(),i1,i2,i3);
	return aux;
}

void SMTFormula::minimize(const intsum & sum){
	hasObjFunc = true;
	isMinimization = true;
	objFunc = sum;
}

void SMTFormula::maximize(const intsum &sum){
	hasObjFunc = true;
	isMinimization = false;
	objFunc = sum;
}

void SMTFormula::setLowerBound(int lb){
	this->LB = lb;
}

void SMTFormula::setUpperBound(int ub){
	this->UB = ub;
}

int SMTFormula::getLowerBound(){
	return this->LB;
}

int SMTFormula::getUpperBound(){
	return this->UB;
}

int SMTFormula::getIValue(const intvar & var, const std::vector<int> & vals){
	return vals[var.id];
}

bool SMTFormula::getBValue(const boolvar & var, const std::vector<bool> & vals){
	return vals[var.id];
}

void SMTFormula::addEmptyClause(){
	clauses.push_back(clause());
}

void SMTFormula::addClause(const clause &c) {
	clauses.push_back(c);
}

void SMTFormula::addSoftClause(const clause &c, int weight) {
	softclauses.push_back(c);
	weights.push_back(weight);
	softclausevars.push_back(intvar());
}

void SMTFormula::addSoftClauseWithVar(const clause &c, int weight, const intvar & var) {
	softclauses.push_back(c);
	weights.push_back(weight);
	softclausevars.push_back(var);
	hassoftclauseswithvars = true;
}

void SMTFormula::addClauses(const std::vector<clause> &c) {
	clauses.insert(clauses.end(),c.begin(),c.end());
}

void SMTFormula::addALO(const std::vector<literal> & v) {
	addClause(v);
}
void SMTFormula::addALOWithCheckVar(const std::vector<literal> &v, boolvar c){
    clause cl(v);
    cl |= c;
    addClause(cl);
}

void SMTFormula::addAMO(const std::vector<literal> & x, AMOEncoding enc) {

	int n = x.size();
	if(n <= 1) //Constraint trivially satisfied
		return;

	switch(enc){
		//Quadratic endoding
		//No auxilliary variables required
		//O(n^2) clauses
		case AMO_QUAD:
			//Mutual exclusion between every pair of variables
			for (int i=0;i<n-1;i++)
				for (int j=i+1;j<n;j++)
					addClause(!x[i] | !x[j]);
			break;

		//Logarithmic endoding
		//O(log(n)) auxilliary variables
		//O(n log(n)) clauses
		case AMO_LOG:
		{
			//Number of bits necessary to represent 0..n-1 in binary
			int m = (int)ceil(log2(n));

			//'y' is the binary representation of the integer 'i' such that variable x[i] is true
			//if more than one variable x[i] is true, a conflict arise
			std::vector<boolvar> y(m);
			for(int j = 0; j < m; j++)
				y[j] = newBoolVar();

			for(int i = 0; i < n; i++)
				for(int j = 0; j < m; j++)
					if((i >> j) % 2 == 0) //If the j-th bit of the binary representation of 'i' is 0
						addClause(!x[i] | !y[j]); //The j-th bit of y has to be 0
					else
						addClause(!x[i] | y[j]); //The j-th bit of y has to be 1
		}
			break;

		//Ladder endoding
		//O(n) auxilliary variables
		//O(n) clauses
		case AMO_LADDER:
		{
			//'y' is an order encoding of the integer 'i' such that the variable x[i] which is true
			//if more than one variable x[i] is true, a conflict arise
			std::vector<boolvar> y(n-1);
			for(int i = 0; i < n-1; i++){
				y[i] = newBoolVar();
				//Order encoding of 'y'
				if(i > 0)
					addClause(!y[i] | y[i-1]);
			}

			for(int i = 0; i < n; i++){

				//Channeling between 'i' and 'y':  x[i] -> (y[i-1] /\ ¬y[i])
				if(i == 0){
					addClause(!x[i] | !y[i]);
					//addClause(x[i] | y[i]); //Needed in EO
				}
				else if(i == n-1){
					addClause(!x[i] | y[i-1]);
					//addClause(x[i] | !y[i-1]); //Needed in EO
				}
				else{
					addClause(!x[i] | y[i-1]);
					addClause(!x[i] | !y[i]);
					//addClause(!y[i-1] | y[i] | x[i]); //Needed in EO
				}
			}
		}
			break;

		//Heule endoding
		//O(n) auxilliary variables
		//O(n) clauses
		case AMO_HEULE:
		{
			if(n <= 3)
				addAMO(x,AMO_QUAD);
			else{
				//Recursively encode AMO(x1,x2,y) /\ AMO(x3,x4,...,xn,¬y)
				boolvar y = newBoolVar();
				std::vector<literal> v1(x.begin(),x.begin()+2);
				std::vector<literal> v2(x.begin()+2,x.end());
				v1.push_back(y);
				v2.push_back(!y);
				addAMO(v1,AMO_QUAD); //AMO(x1,x2,y)
				addAMO(v2,AMO_LADDER); //AMO(x3,x4,...,xn,¬y)
			}
		}
			break;


		//Commander encoding with k=3
		//O(n) auxilliary variables
		//O(n) clauses
		case AMO_COMMANDER:
		{
			if(n<6)
				addAMO(x,AMO_QUAD);
			else{
				int nsplits = n/3;
				if(n%3!=0) nsplits++;

				std::vector<literal> cmd_vars(nsplits);

				if(nsplits==2){
					cmd_vars[0] = newBoolVar();
					cmd_vars[1] = !cmd_vars[0];
				}
				else{
					for(int i= 0; i < nsplits; i++)
						cmd_vars[i] = newBoolVar();
					addEO(cmd_vars,AMO_COMMANDER);
				}

				for(int i = 0; i < nsplits; i++){
					std::vector<literal> v;
					for(int j = 3*i; j < 3*(i+1) && j < n; j++){
						v.push_back(x[j]);
						addClause(v[i] | !x[j]);
					}
					addAMO(v,AMO_QUAD);
				}
			}
		}
		default:
			std::cerr << "Wrong AMO encoding" << std::endl;
			exit(BADCODIFICATION_ERROR);
			break;
	}
}


void SMTFormula::addAMOWithCheckVar(const std::vector<literal> &x, boolvar c, AMOEncoding enc) {

    int n = x.size();
    if(n <= 1) //Constraint trivially satisfied
        return;

    switch(enc){
        //Quadratic endoding
        //No auxilliary variables required
        //O(n^2) clauses
        case AMO_QUAD:
            //Mutual exclusion between every pair of variables
            for (int i=0;i<n-1;i++)
                for (int j=i+1;j<n;j++)
                    addClause(!x[i] | !x[j] | c);
            break;

        //Logarithmic endoding
        //O(log(n)) auxilliary variables
        //O(n log(n)) clauses
        case AMO_LOG:
        {
            //Number of bits necessary to represent 0..n-1 in binary
            int m = (int)ceil(log2(n));

            //'y' is the binary representation of the integer 'i' such that variable x[i] is true
            //if more than one variable x[i] is true, a conflict arise
            std::vector<boolvar> y(m);
            for(int j = 0; j < m; j++)
                y[j] = newBoolVar();

            for(int i = 0; i < n; i++)
                for(int j = 0; j < m; j++)
                    if((i >> j) % 2 == 0) //If the j-th bit of the binary representation of 'i' is 0
                        addClause(!x[i] | !y[j] | c); //The j-th bit of y has to be 0
                    else
                        addClause(!x[i] | y[j] | c); //The j-th bit of y has to be 1
        }
            break;

        //Ladder endoding
        //O(n) auxilliary variables
        //O(n) clauses
        case AMO_LADDER:
        {
            //'y' is an order encoding of the integer 'i' such that the variable x[i] which is true
            //if more than one variable x[i] is true, a conflict arise
            std::vector<boolvar> y(n-1);
            for(int i = 0; i < n-1; i++){
                y[i] = newBoolVar();
                //Order encoding of 'y'
                if(i > 0)
                    addClause(!y[i] | y[i-1] | c);
            }

            for(int i = 0; i < n; i++){

                //Channeling between 'i' and 'y':  x[i] -> (y[i-1] /\ ¬y[i])
                if(i == 0){
                    addClause(!x[i] | !y[i] | c);
                    //addClause(x[i] | y[i]); //Needed in EO
                }
                else if(i == n-1){
                    addClause(!x[i] | y[i-1] | c);
                    //addClause(x[i] | !y[i-1]); //Needed in EO
                }
                else{
                    addClause(!x[i] | y[i-1] | c);
                    addClause(!x[i] | !y[i] | c);
                    //addClause(!y[i-1] | y[i] | x[i]); //Needed in EO
                }
            }
        }
            break;

        //Heule endoding
        //O(n) auxilliary variables
        //O(n) clauses
        case AMO_HEULE:
        {
            if(n <= 3)
                addAMOWithCheckVar(x,c,AMO_QUAD);
            else{
                //Recursively encode AMO(x1,x2,y) /\ AMO(x3,x4,...,xn,¬y)
                boolvar y = newBoolVar();
                std::vector<literal> v1(x.begin(),x.begin()+2);
                std::vector<literal> v2(x.begin()+2,x.end());
                v1.push_back(y);
                v2.push_back(!y);
                addAMOWithCheckVar(v1,c,AMO_QUAD); //AMO(x1,x2,y)
                addAMOWithCheckVar(v2,c,AMO_LADDER); //AMO(x3,x4,...,xn,¬y)
            }
        }
            break;


        //Commander encoding with k=3
        //O(n) auxilliary variables
        //O(n) clauses
        case AMO_COMMANDER:
        {
            if(n<6)
                addAMOWithCheckVar(x,c,AMO_QUAD);
            else{
                int nsplits = n/3;
                if(n%3!=0) nsplits++;

                std::vector<literal> cmd_vars(nsplits);

                if(nsplits==2){
                    cmd_vars[0] = newBoolVar();
                    cmd_vars[1] = !cmd_vars[0];
                }
                else{
                    for(int i= 0; i < nsplits; i++)
                        cmd_vars[i] = newBoolVar();
                    addEOWithCheckVar(cmd_vars,c,AMO_COMMANDER);
                }

                for(int i = 0; i < nsplits; i++){
                    std::vector<literal> v;
                    for(int j = 3*i; j < 3*(i+1) && j < n; j++){
                        v.push_back(x[j]);
                        addClause(v[i] | !x[j] | c);
                    }
                    addAMOWithCheckVar(v,c,AMO_QUAD);
                }
            }
        }
        default:
            std::cerr << "Wrong AMO encoding" << std::endl;
            exit(BADCODIFICATION_ERROR);
            break;
    }
}


void SMTFormula::addEO(const std::vector<literal> & v, AMOEncoding enc) {
	addALO(v);
	addAMO(v,enc);
}
void SMTFormula::addEOWithCheckVar(const std::vector<literal> &v, boolvar c, AMOEncoding enc) {
    addALOWithCheckVar(v,c);
    addAMOWithCheckVar(v,c,enc);
}

void SMTFormula::addALK(const std::vector<literal> & v, int K){
	int n = v.size();
	if(K>n){ //Trivially false
		addEmptyClause();
		return;
	}
	else if(K<=0)//Trivially true
		return;
	else if(K==1){ //At least one constraint
		addALO(v);
	}
	else if(n==K)
		for(const literal &l : v)
			addClause(l);
	else{
		if(K > n/2){ //AMK encoding of the negated literals is smaller
			std::vector<literal> v2;
			for(const literal & l : v)
				v2.push_back(!l);
			addAMK(v2,n-K,CARD_SORTER);
		}
		else{
			std::vector<literal> sorted;
			addMCardinality(v,sorted,K,false,true);
			addClause(sorted[K-1]);
		}
	}
}
void SMTFormula::addALKWithCheckVar(const vector<literal> &v, int K, boolvar c){
    int n = v.size();
    if(K>n){ //Trivially false
        addEmptyClause();
        return;
    }
    else if(K<=0)//Trivially true
        return;
    else if(K==1){ //At least one constraint
        addALOWithCheckVar(v,c);
    }
    else if(n==K)
        for(const literal &l : v)
            addClause(l | c);
    else{
        if(K > n/2){ //AMK encoding of the negated literals is smaller
            std::vector<literal> v2;
            for(const literal & l : v)
                v2.push_back(!l);
            addAMKWithCheckVar(v2,n-K,c,CARD_SORTER);
        }
        else{
            std::vector<literal> sorted;
            addMCardinality(v,sorted,K,false,true);
            addClause(sorted[K-1]|c);
        }
    }
}
void SMTFormula::addAMK(const std::vector<literal> & v, int K, CardinalityEncoding enc){
	int n = v.size();
	if(K < 0){ //Trivially false
		addEmptyClause();
		return;
	}
	else if(K>=n){ //Trivially true
		return;
	}
	else if(K==0) //All must be false
		for(const literal &l : v)
			addClause(!l);
	else if(K==1){ //At most one constraint
		if(n >= 5)
			addAMO(v,AMO_LADDER);
		else
			addAMO(v,AMO_QUAD);
	}
	else{
		switch(enc){
			case CARD_TOTALIZER:
			{
				std::vector<literal> root(std::min(K+1,n));
				addTotalizer(v,root,K);
				if(root.size() > K)
					addClause(!root[K]);
			}
			break;

			case CARD_SORTER:
			default:
			{
				if(K > n/2){ //ALK encoding of the negated literals is smaller
					std::vector<literal> v2;
					for(const literal & l : v)
						v2.push_back(!l);
					addALK(v2,n-K);
				}
				else{
					std::vector<literal> sorted;
					addMCardinality(v,sorted,K+1,true,false);
					addClause(!sorted[K]);
				}
			}
			break;
		}
	}
}
void SMTFormula::addAMKWithCheckVar(const vector<literal> &v, int K, boolvar c, CardinalityEncoding enc){
    int n = v.size();
    if(K < 0){ //Trivially false
        addEmptyClause();
        return;
    }
    else if(K>=n){ //Trivially true
        return;
    }
    else if(K==0) //All must be false
        for(const literal &l : v)
            addClause(!l | c);
    else if(K==1){ //At most one constraint
        if(n >= 5)
            addAMOWithCheckVar(v,c,AMO_LADDER);
        else
            addAMOWithCheckVar(v,c,AMO_QUAD);
    }
    else{
        switch(enc){
            case CARD_TOTALIZER:
            {
                std::vector<literal> root(std::min(K+1,n));
                addTotalizer(v,root,K);
                if(root.size() > K)
                    addClause(!root[K] | c);
            }
            break;

            case CARD_SORTER:
            default:
            {
                if(K > n/2){ //ALK encoding of the negated literals is smaller
                    std::vector<literal> v2;
                    for(const literal & l : v)
                        v2.push_back(!l);
                    addALKWithCheckVar(v2,n-K,c);
                }
                else{
                    std::vector<literal> sorted;
                    addMCardinality(v,sorted,K+1,true,false);
                    addClause(!sorted[K] | c);
                }
            }
            break;
        }
    }
}
void SMTFormula::addEK(const std::vector<literal> & v, int K){
	int n = v.size();
	if(K>n | K < 0){ //Triviavlly false
		addEmptyClause();
		return;
	}
	else if(K == n) //All must be true
		for(const literal &l : v)
			addClause(l);
	else if(K==0) //All must be false
		for(const literal &l : v)
			addClause(!l);
	else{
		if(K > n/2){ //EK encoding of the negated literals is smaller
			std::vector<literal> v2;
			for(const literal & l : v)
				v2.push_back(!l);
			addEK(v2,n-K);
		}
		else{
			std::vector<literal> sorted(n);
			addMCardinality(v,sorted,K+1,true,true);
			addClause(sorted[K-1]);
			addClause(!sorted[K]);
		}
	}
}
void SMTFormula::addEKWithCheckVar(const vector<literal> &v, int K, boolvar c){
    int n = v.size();
    if(K>n | K < 0){ //Triviavlly false
        addEmptyClause();
        return;
    }
    else if(K == n) //All must be true
        for(const literal &l : v)
            addClause(l|c);
    else if(K==0) //All must be false
        for(const literal &l : v)
            addClause(!l|c);
    else{
        if(K > n/2){ //EK encoding of the negated literals is smaller
            std::vector<literal> v2;
            for(const literal & l : v)
                v2.push_back(!l);
            addEKWithCheckVar(v2,n-K,c);
        }
        else{
            std::vector<literal> sorted(n);
            addMCardinality(v,sorted,K+1,true,true);
            addClause(sorted[K-1] | c);
            addClause(!sorted[K] | c);
        }
    }
}
void SMTFormula::addPB(const std::vector<int> & Q, const std::vector<literal> & X, int K, PBEncoding encoding){

	std::vector<std::vector<int> > QQ;
	std::vector<std::vector<literal> > XX;
	switch(encoding){
		case PB_BDD:
		{
			std::vector<int> Q2 = Q;
			std::vector<literal> X2 = X;
			util::sortCoefsDecreasing(Q2,X2);
			AMOPBMDDBuilder mb(Q2,X2,K);
			MDD * mdd = mb.getMDD();
			addClause(assertMDDLEQAbio(mdd));
		}
			break;

		case PB_SWC:
			util::PBtoAMOPB(Q,X,QQ,XX);
			addAMOPB(QQ,XX,K,AMOPB_SWC);
			break;

		case PB_GT:
			util::PBtoAMOPB(Q,X,QQ,XX);
			addAMOPB(QQ,XX,K,AMOPB_GT);
			break;

		case PB_LPW:
			util::PBtoAMOPB(Q,X,QQ,XX);
			addAMOPB(QQ,XX,K,AMOPB_LPW);
			break;

		case PB_GPW:
			util::PBtoAMOPB(Q,X,QQ,XX);
			addAMOPB(QQ,XX,K,AMOPB_GPW);
			break;

		case PB_LBM:
			util::PBtoAMOPB(Q,X,QQ,XX);
			addAMOPB(QQ,XX,K,AMOPB_LBM);
			break;

		case PB_GBM:
			util::PBtoAMOPB(Q,X,QQ,XX);
			addAMOPB(QQ,XX,K,AMOPB_GBM);
			break;

		default:
			std::cerr << "Wrong kind of PB encoding" << std::endl;
			exit(BADCODIFICATION_ERROR);
			break;
	}
}

void SMTFormula::addAMOPB(const std::vector<std::vector<int> > & Q, const std::vector<std::vector<literal> > & X, int K, AMOPBEncoding encoding){

	std::map<AMOPBEncoding,PBEncoding>::iterator it = amopb_pb_rel.find(encoding);
	if(it != amopb_pb_rel.end()){
		std::vector<int> Q2;
		std::vector<literal> X2;
		util::AMOPBtoPB(Q,X,Q2,X2);
		addPB(Q2,X2,K,it->second);
		return;
	}


	if(X.size()!=Q.size())
	{
		std::cerr << "Tried to assert an AMO-PB with different number of coefficient groups and variable groups" << std::endl;
		exit(BADCODIFICATION_ERROR);
	}

	std::vector<std::vector<int> > Q2;
	std::vector<std::vector<literal> > X2;

	int maxsum = 0;

	for(int i = 0; i < X.size(); i++){
		if(X[i].size()!=Q[i].size()){
			std::cerr << "Tried to assert an AMO-PB with different number of coefficients and variables in a group" << std::endl;
			exit(BADCODIFICATION_ERROR);
		}

		int max = 0;
		std::vector<int> q;
		std::vector<literal> x;
		for(int j = 0; j < X[i].size(); j++){
			if(Q[i][j]>K)
				addClause(!X[i][j]);
			else if(Q[i][j] > 0){
				q.push_back(Q[i][j]);
				x.push_back(X[i][j]);
				if(Q[i][j] > max)
					max = Q[i][j];
			}
		}
		if(!q.empty()){
			Q2.push_back(q);
			X2.push_back(x);
		}
		maxsum+=max;
	}

	//Trivial cases
	if(K < 0){
		addEmptyClause();
		return;
	}
	if(K==0){
		for(const std::vector<literal> & v : X2)
			for(const literal & l : v)
				addClause(!l);
		return;
	}
	if(K >= maxsum)
		return;

	int N = X2.size();
	if(N==1) //We have enforced that no AMO group contains a coefficient greater than K
		return;

	switch(encoding){
		case AMOPB_AMOMDD:
		{
			util::sortCoefsDecreasing(Q2,X2);

			AMOPBMDDBuilder mb(Q2,X2,K);
			MDD * mdd = mb.getMDD();
			addClause(assertMDDLEQAbio(mdd));
		}
			break;

		case AMOPB_IMPCHAIN :
		{
			std::vector<std::vector<literal> > Y2;
			for(int i = 0; i < X2.size();i++){
				Y2.push_back(std::vector<literal>(X2[i].size()));
				for(int j = 0; j < X2[i].size(); j++)
					Y2[i][j] = newBoolVar();
				util::sortCoefsIncreasing(Q2[i],X2[i]);
				for(int j = 0; j < X2[i].size(); j++){
					addClause(!X2[i][j]|Y2[i][j]);
					if(j < X2[i].size()-1){
						addClause(!X2[i][j] | !Y2[i][j+1]);
						addClause(!Y2[i][j+1] | Y2[i][j]);
					}
				}

			}

			AMOPBMDDBuilder mb(Q2,Y2,K);
			MDD * mdd = mb.getMDD();
			addClause(assertMDDLEQAbio(mdd));
		}
			break;

		case AMOPB_AMOBDD:
		{
			AMOPBBDDBuilder mb(Q2,X2,K);
			MDD * mdd = mb.getMDD();
			addClause(assertMDDLEQAbio(mdd));
		}
			break;

		case AMOPB_SWC:
			addAMOPBSWC(Q2,X2,K);
			break;

		case AMOPB_SORTER:
			addAMOPBSorter(Q2,X2,K);
			break;

		case AMOPB_GT:
			addAMOPBGeneralizedTotalizer(Q2,X2,K);
			break;

		case AMOPB_GBM:
			addAMOPBGlobalPolynomialWatchdog(Q2,X2,K,true);
			break;

		case AMOPB_LBM:
			addAMOPBLocalPolynomialWatchdog(Q2,X2,K,true);
			break;

		case AMOPB_LPW:
			addAMOPBLocalPolynomialWatchdog(Q2,X2,K,false);
			break;

		case AMOPB_GPW:
			addAMOPBGlobalPolynomialWatchdog(Q2,X2,K,false);
			break;

		default://PB encoding
			std::cerr << "Wrong kind of AMOPB encoding" << std::endl;
			exit(SOLVING_ERROR);
			break;
	}
}


void SMTFormula::addPBGEQ(const std::vector<int> & Q, const std::vector<literal> & X, int K, PBEncoding encoding){

	std::vector<literal> X2;
	int K2=0;

	for(int i = 0; i < Q.size(); i++){
		X2[i]=!X[i];
		K2+=Q[i];
	}
	K2 -= K;

	addPB(Q,X2,K,encoding);
}


void SMTFormula::addAMOPBGEQ(const std::vector<std::vector<int> > & Q, const std::vector<std::vector<literal> > & X, int K, AMOPBEncoding encoding){

	std::vector<std::vector<int> > Q2(Q.size());
	std::vector<std::vector<literal> > X2(X.size());
	int K2=-K;

	for(int i = 0; i < Q.size(); i++){
		if(Q[i].size()==1){
			X2[i].push_back(!X[i][0]);
			Q2[i].push_back(Q[i][0]);
			K2+=Q[i][0];
		}
		else{
			boolvar aux = newBoolVar();
			int maxq = *max_element(Q[i].begin(),Q[i].end());
			std::vector<literal> c;
			c.push_back(aux);
			X2[i].push_back(aux);
			Q2[i].push_back(maxq);
			K2+=maxq;
			for(int j = 0; j < Q[i].size(); j++){
				int q = maxq-Q[i][j];
				addClause(!aux | !X[i][j]);
				c.push_back(X[i][j]);
				if(q > 0){
					Q2[i].push_back(q);
					X2[i].push_back(X[i][j]);
				}
			}
			addClause(c);
		}
	}

	addAMOPB(Q2,X2,K2,encoding);
}




literal SMTFormula::assertMDDLEQAbio(MDD * mdd) {
	std::vector<literal> asserted(mdd->getId()+1);
	boolvar undef;
	undef.id=-1;

	for(int i = 0; i <= mdd->getId(); i++)
		asserted[i] = undef;
	literal rootvar = assertMDDLEQAbio(mdd,asserted);
	return rootvar;
}

literal SMTFormula::assertMDDLEQAbio(MDD * mdd, std::vector<literal> & asserted) {
	if(mdd->isTrueMDD())
		return trueVar();
	else if(mdd->isFalseMDD())
		return falseVar();

	literal v=asserted[mdd->getId()];
	if(v.v.id==-1){
		v = newBoolVar();
		asserted[mdd->getId()]=v;

		literal velse = assertMDDLEQAbio(mdd->getElseChild(),asserted);
		addClause(velse | !v);

		for(int i = 0; i < mdd->getNSelectors();i++){
			std::pair<literal,MDD *> p = mdd->getSelectorAndChild(i);
			if(p.second != mdd->getElseChild())
			{
				literal vi = assertMDDLEQAbio(p.second,asserted);
				addClause(vi | !p.first | !v);
			}
		}
	}

	return v;
}


void SMTFormula::addAMOPBSWC(const std::vector<std::vector<int> > & Q, const std::vector<std::vector<literal> > & X, int K){

	int N = X.size();

	std::vector<literal> Sin, Sout(K);

	for(int i = 0; i < N; i++){
		Sin = Sout;

		if(i < N-1)
			for(int j = 0; j < K; j++)
				Sout[j] = newBoolVar();

		if(i < N-1 && i > 0)
			for(int j = 0; j < K; j++)
				addClause(!Sin[j] | Sout[j]);

		for(int j = 0; j < X[i].size(); j++){
			int q = Q[i][j];
			if(i < N-1)
				for(int k = 0; k < q; k++)
					addClause(!X[i][j] | Sout[k]);
			if(i>0 && i < N-1)
				for(int k = 0; k < K-q; k++)
					addClause(!Sin[k] | !X[i][j] | Sout[k+q]);
			if(i>0 && q > 0)
				addClause(!Sin[K-q] | !X[i][j]);
		}
	}
}

void SMTFormula::addAMOPBSorter(const std::vector<std::vector<int> > & Q, const std::vector<std::vector<literal> > & X, int K){

	int n = Q.size();

	if(K==0){
		for(const std::vector<literal> & v : X)
			for(const literal & l : v)
				addClause(!l);
		return;
	}

	int maxsum = 0;
	for(int i = 0; i < n; i++)
		maxsum+= *(max_element(Q[i].begin(),Q[i].end()));

	if(maxsum <= K)
		return;

	std::vector<std::vector<literal> > orders;
	for(int i = 0; i < n; i++){
		int maxq = *(max_element(Q[i].begin(),Q[i].end()));
		if(maxq > 0){
			std::vector<literal> order;
			addOrderEncoding(maxq,order);
			for(int j = 0; j < Q[i].size(); j++)
				if(Q[i][j]>0)
					addClause(!X[i][j] | order[Q[i][j]-1]);
			orders.push_back(order);
		}
	}

	while(orders.size()>1){
		sort(orders.begin(), orders.end(),
          [](const std::vector<literal>& a, const std::vector<literal>& b) {
					return a.size() < b.size();
				}
		);
		std::vector<literal> result;
		addSimplifiedMerge(orders[0],orders[1],result,K+1,true,false);
		orders.push_back(result);
		orders.erase(orders.begin());
		orders.erase(orders.begin());
	}

	addClause(!orders[0][K]);
}


inline int lchild(int i){
	return 2*i+1;
}

inline int rchild(int i){
	return 2*i+2;
}

inline int parent(int i){
	return (i-1)/2;
}

void SMTFormula::addAMOPBGeneralizedTotalizer(const std::vector<std::vector<int> > & Q, const std::vector<std::vector<literal> > & X, int K){

	int n = X.size();

	std::vector<int> * tree = new std::vector<int>[2*n-1];
	std::vector<literal> * treevars = new std::vector<literal> [2*n-1];
	std::map<int,literal> * treevarsmap = new std::map<int,literal> [n-1];

	//Fill tree nodes with coefficients
	for(int i = 0; i < n; i++){
		int idx = n-1+i;
		tree[idx].clear();
		treevars[idx].clear();
		std::map<int,int> count;
		std::map<int,literal> lit;
		for(int j = 0; j < Q[i].size(); j++){

			int q = Q[i][j];
			if(q!=0){
				if(count.find(q) == count.end()){
					count[q] = 1;
					lit[q] = X[i][j];
				}
				else{
					if(count[q]==1){
						boolvar v = newBoolVar();
						addClause(!lit[q] | v);
						lit[q] = v;
					}
					addClause(!X[i][j] | lit[q]);
					count[q] = count[q]+1;
				}
			}
		}
		for(const std::pair<int,literal> & p : lit){
			tree[idx].push_back(p.first);
			treevars[idx].push_back(p.second);
		}
		tree[idx].push_back(0);
	}

	for(int i = n-2; i >= 0; i--){
		tree[i].clear();
		for(int j = 0; j < tree[lchild(i)].size(); j++){
			for(int k = 0; k < tree[rchild(i)].size(); k++){
				int x = std::min(tree[lchild(i)][j]+tree[rchild(i)][k],K+1);
				if(!util::existsSorted(tree[i],x))
					util::insertSorted(tree[i],x);
			}
		}
	}

	//Simplify the tree with the unnecessary values

	//Simplify the root
	if(tree[0][0] < K+1)
		return;

	tree[0].resize(2);
	tree[0][1]=0;

	//Encode the tree
	for(int i = n-2; i >= 0; i--){
		for(int j = 0; j < tree[i].size()-1; j++){
			boolvar v = newBoolVar();
			treevars[i].push_back(v);
			treevarsmap[i][tree[i][j]] = v;
		}

		int l = lchild(i);
		int r = rchild(i);

		for(int j = 0; j < tree[l].size(); j++){
			for(int k = 0; k < tree[r].size(); k++){
				int x = std::min(tree[l][j]+tree[r][k],K+1);
				if(treevarsmap[i].find(x) != treevarsmap[i].end()){
					if(j<tree[l].size()-1 || k < tree[r].size()-1){
						if(j == tree[l].size()-1)
							addClause(!treevars[r][k] | treevarsmap[i][x]);
						else if(k == tree[r].size()-1)
							addClause(!treevars[l][j] | treevarsmap[i][x]);
						else
							addClause(!treevars[l][j] | !treevars[r][k] | treevarsmap[i][x]);
					}
				}
			}
		}
	}


	//Negate that the sum is greater than K
	if(!tree[0].empty())
		addClause(!treevars[0][0]);

	delete [] tree;
	delete [] treevars;
	delete [] treevarsmap;
}


literal SMTFormula::addPolynomialWatchdog(const std::vector<std::vector<int> > & Q, const std::vector<std::vector<literal> > & X, int K, bool useSorter){
	int n = Q.size();

	//This encoding is for < K instead of <= K
	K+=1;

	int max = 0;
	for(const std::vector<int> & q : Q)
		for(int qi : q)
			if(qi > max)
				max = qi;

	int p = (int)floor(log2(max));
	int p2 = (int) exp2(p);
	int m = K / p2;
	if(K%p2 != 0)
		m++;
	int T = (m*p2) - K;

	std::vector<std::vector<literal> > B(p+1); //Buckets

	for(int k = 0; k <= p; k++){
		for(int i = 0; i < n; i++){
			bool used = false;
			bool created = false;
			literal vk;
			for(int j = 0; j < Q[i].size(); j++){
				if(util::nthBit(Q[i][j],k)){
					if(!used){
						vk = X[i][j];
						used = true;
					}
					else{
						if(!created){
							boolvar aux = newBoolVar();
							addClause(!vk | aux);
							vk = aux;
							created=true;
						}
						addClause(!X[i][j] | vk);
					}
				}
			}
			if(used)
				B[k].push_back(vk);
		}
	}

	std::vector<literal> S, Shalf;
	for(int i = 0; i <= p; i++){
		S.clear();
		std::vector<literal> U;
		if(useSorter)
			addSorting(B[i],U,true,false);
		else
			addTotalizer(B[i],U);
		if(i==0)
			S=U;
		else{
			if(useSorter)
				addMerge(U,Shalf,S,true,false);
			else
				addQuadraticMerge(U,Shalf,S);
		}
		if(util::nthBit(T,i))
			S.insert(S.begin(),trueVar());

		Shalf.resize(S.size()/2);
		for(int i = 1; i < S.size(); i+=2)
			Shalf[i/2]=S[i];
	}
	return S[m-1];
}


void SMTFormula::addAMOPBGlobalPolynomialWatchdog(const std::vector<std::vector<int> > & Q, const std::vector<std::vector<literal> > & X, int K, bool useSorter){
	if(K==0){
		for(const std::vector<literal> & v : X)
			for(const literal & l : v)
				addClause(!l);
		return;
	}
	else if(K < 0){
		addEmptyClause();
		return;
	}
	int n = Q.size();

	int maxsum = 0;
	for(int i = 0; i < n; i++)
		maxsum+= *(max_element(Q[i].begin(),Q[i].end()));

	if(maxsum <= K)
		return;

	if(n==1){
		clause c;
		for(int i = 0; i < Q[0].size(); i++)
			if(Q[0][i] >= K)
				c |= X[0][i];
		addClause(c);
		return;
	}

	addClause(!addPolynomialWatchdog(Q,X,K,useSorter));
}

void SMTFormula::addAMOPBLocalPolynomialWatchdog(const std::vector<std::vector<int> > & Q, const std::vector<std::vector<literal> > & X, int K, bool useSorter){
	if(K==0){
		for(const std::vector<literal> & v : X)
			for(const literal & l : v)
				addClause(!l);
		return;
	}
	else if(K < 0){
		addEmptyClause();
		return;
	}
	int n = Q.size();

	int maxsum = 0;
	for(int i = 0; i < n; i++)
		maxsum+= *(max_element(Q[i].begin(),Q[i].end()));

	if(maxsum <= K)
		return;

	if(n==1){
		clause c;
		for(int i = 0; i < Q[0].size(); i++)
			if(Q[0][i] >= K)
				c |= X[0][i];
		addClause(c);
		return;
	}

	for(int i = 0; i < n; i++){
		std::vector<std::vector<int> > Q2 = Q;
		std::vector<std::vector<literal> > X2 = X;
		Q2[i]=Q2[Q2.size()-1];
		Q2.resize(Q2.size()-1);
		X2[i]=X2[X2.size()-1];
		X2.resize(X2.size()-1);
		for(int j = 0; j < Q[i].size(); j++){
			std::map<int,literal> watchdog;
			if(watchdog.find(Q[i][j]) == watchdog.end())
				watchdog[Q[i][j]] = addPolynomialWatchdog(Q2,X2,K-Q[i][j],useSorter);
			addClause(!watchdog[Q[i][j]]| !X[i][j]);
		}
	}
}





void SMTFormula::addOrderEncoding(int x, std::vector<literal> & lits){

	lits.resize(x);
	lits[0]=newBoolVar();
	for(int i = 1; i < x; i++){
		lits[i] = newBoolVar();
		addClause(!lits[i] | lits[i-1]);
	}
}



void SMTFormula::addTwoComparator(const literal &x1, const literal &x2, literal &y1, literal &y2, bool leqclauses, bool geqclauses){
	y1 = newBoolVar();
	y2 = newBoolVar();

	if(leqclauses){
		addClause(!x1 | y1);
		addClause(!x2 | y1);
		addClause(!x1 | ! x2 | y2);
	}

	if(geqclauses){
		addClause(x1 | !y2);
		addClause(x2 | !y2);
		addClause(x1 | x2 | !y1);
	}
}

void SMTFormula::addQuadraticMerge(const std::vector<literal> &x1, const std::vector<literal> &x2, std::vector<literal> &y){
	if(x1.empty())
		y = x2;
	else if(x2.empty())
		y = x1;
	else{
		y.resize(x1.size() + x2.size());
		for(int i = 0; i < x1.size() + x2.size(); i++)
			y[i] = newBoolVar();
		for(int i = 0; i < x1.size(); i++){
			addClause(!x1[i] | y[i]);
			for(int j = 0; j < x2.size(); j++)
				addClause(!x1[i] | !x2[j] | y[i+j+1]);
		}
		for(int i = 0; i < x2.size(); i++)
			addClause(!x2[i] | y[i]);
	}
}

void SMTFormula::addMerge(const std::vector<literal> &x1, const std::vector<literal> &x2, std::vector<literal> &y, bool leqclauses, bool geqclauses){
	int a = x1.size();
	int b = x2.size();

	if(a==0 && b==0){
		y.clear();
		return;
	}

	y.resize(a+b);

	if(a==1 && b==1)
		addTwoComparator(x1[0],x2[0],y[0],y[1],leqclauses,geqclauses);
	else if(a == 0)
		y = x2;
	else if(b == 0)
		y = x1;
	else{
		std::vector<literal> x1even, x1odd, x2even, x2odd;
		for(int i = 0; i < a-1; i+=2){
				x1even.push_back(x1[i]);
				x1odd.push_back(x1[i+1]);
		}
		if(a%2==1)
			x1even.push_back(x1[a-1]);

		for(int i = 0; i < b-1; i+=2){
				x2even.push_back(x2[i]);
				x2odd.push_back(x2[i+1]);
		}
		if(b%2==1)
			x2even.push_back(x2[b-1]);

		std::vector<literal> zeven;
		std::vector<literal> zodd;

		addMerge(x1even, x2even, zeven,leqclauses,geqclauses);
		addMerge(x1odd, x2odd,zodd,leqclauses,geqclauses);

		std::vector<literal> z(a+b);
		if(a%2==0){
			if(b%2==0){
				for(int i = 0; i < (a+b)/2; i++)
					z[2*i] = zeven[i];

				for(int i = 0; i < (a+b)/2; i++)
					z[2*i + 1] = zodd[i];

				y[0] = z[0];
				y[a+b-1] = z[a+b-1];
				for(int i = 1; i < a+b-2; i+=2)
					addTwoComparator(z[i],z[i+1],y[i],y[i+1],leqclauses,geqclauses);

			}else{
				for(int i = 0; i < (a+b)/2 + 1; i++)
					z[2*i] = zeven[i];

				for(int i = 0; i < (a+b)/2; i++)
					z[2*i + 1] = zodd[i];

				y[0] = z[0];
				for(int i = 1; i < a+b-1; i+=2)
					addTwoComparator(z[i],z[i+1],y[i],y[i+1],leqclauses,geqclauses);

			}
		}
		else{ //a%2==1
			if(b%2==0)
				addMerge(x2,x1,y,leqclauses,geqclauses);
			else{
				for(int i = 0; i < (a+1)/2; i++)
					z[2*i] = zeven[i];
				for(int i = 0; i < (b+1)/2; i++)
					z[a + 2*i] = zeven[(a+1)/2 + i];

				for(int i = 0; i < a/2; i++)
					z[2*i+1] = zodd[i];
				for(int i = 0; i < b/2; i++)
					z[a + 2*i+1] = zodd[a/2 + i];

				y[0] = z[0];
				y[a+b-1] = z[a+b-1];
				for(int i = 1; i < a+b-2; i+=2)
					addTwoComparator(z[i],z[i+1],y[i],y[i+1],leqclauses,geqclauses);

			}
		}
	}
}


void SMTFormula::addSimplifiedMerge(const std::vector<literal> &x1, const std::vector<literal> &x2, std::vector<literal> &y, int c, bool leqclauses, bool geqclauses){
	int a = x1.size();
	int b = x2.size();

	if(a==0 && b==0){
		y.clear();
		return;
	}

	if(c==0){
		y.clear();
		return;
	}


	if(a==1 && b==1 && c==1){
		y.resize(c);
		y[0] = newBoolVar();
		if(leqclauses){
			addClause(!x1[0] | y[0]);
			addClause(!x2[0] | y[0]);
		}
		if(geqclauses)
			addClause(x1[0] | x2[0] | !y[0]);
	}
	else if(a > c)
		addSimplifiedMerge(std::vector<literal>(x1.begin(),x1.begin()+c),x2,y,c,leqclauses,geqclauses);
	else if(b > c)
		addSimplifiedMerge(x1,std::vector<literal>(x2.begin(),x2.begin()+c),y,c,leqclauses,geqclauses);
	else if(a+b<=c)
		addMerge(x1,x2,y,leqclauses,geqclauses);
	else{
		y.resize(c);
		std::vector<literal> x1even, x1odd, x2even, x2odd;
		for(int i = 0; i < a-1; i+=2){
				x1even.push_back(x1[i]);
				x1odd.push_back(x1[i+1]);
		}
		if(a%2==1)
			x1even.push_back(x1[a-1]);

		for(int i = 0; i < b-1; i+=2){
				x2even.push_back(x2[i]);
				x2odd.push_back(x2[i+1]);
		}
		if(b%2==1)
			x2even.push_back(x2[b-1]);

		std::vector<literal> zeven;
		std::vector<literal> zodd;
		std::vector<literal> z;

		if(c%2==0){
			addSimplifiedMerge(x1even, x2even, zeven, c/2 + 1,leqclauses,geqclauses);
			addSimplifiedMerge(x1odd, x2odd, zodd, c/2,leqclauses,geqclauses);

			z.resize(c+1);
			for(int i = 0; i < c/2; i++){
				z[2*i] = zeven[i];
				z[2*i +1] = zodd[i];
			}
			z[c] = zeven[c/2];

			y[0] = z[0];
			for(int i = 1; i < c-2; i+=2)
				addTwoComparator(z[i],z[i+1],y[i],y[i+1],leqclauses,geqclauses);
			y[c-1]=newBoolVar();
			if(leqclauses){
				addClause(!z[c-1] | y[c-1]);
				addClause(!z[c] | y[c-1]);
			}
			if(geqclauses)
				addClause(z[c-1] | z[c] | !y[c-1]);

		}
		else{ //c%2==1
			addSimplifiedMerge(x1even, x2even, zeven, (c+1)/2,leqclauses,geqclauses);
			addSimplifiedMerge(x1odd, x2odd, zodd, (c-1)/2,leqclauses,geqclauses);

			z.resize(c);
			for(int i = 0; i < (c-1)/2; i++){
				z[2*i] = zeven[i];
				z[2*i +1] = zodd[i];
			}
			z[c-1] = zeven[(c-1)/2];

			y[0] = z[0];
			for(int i = 1; i < c-1; i+=2)
				addTwoComparator(z[i],z[i+1],y[i],y[i+1],leqclauses,geqclauses);
		}
	}
}

void SMTFormula::addTotalizer(const std::vector<literal> &x, std::vector<literal> &y){

	int n = x.size();

	if(n==0){
		y.clear();
		return;
	}
	if(n==1){
		y = x;
		return;
	}

	std::vector<literal> * tree = new std::vector<literal> [2*n-1];

	//Fill tree nodes with coefficients
	for(int i = 0; i < n; i++){
		int idx = n-1+i;
		tree[idx].resize(1);
		tree[idx][0] = x[i];
	}

	for(int i = n-2; i >= 0; i--){
		int ls = tree[lchild(i)].size();
		int rs = tree[rchild(i)].size();
		tree[i].resize(ls + rs);
		for(int j = 0; j < ls + rs; j++)
			tree[i][j] = newBoolVar();

		for(int j = 0; j < ls; j++){
			addClause(!tree[lchild(i)][j] | tree[i][j]);
			for(int k = 0; k < rs; k++)
				addClause(!tree[lchild(i)][j] | !tree[rchild(i)][k] | tree[i][j+k+1]);
		}
		for(int k = 0; k < rs; k++)
			addClause(!tree[rchild(i)][k] | tree[i][k]);
	}

	y = tree[0];

	delete [] tree;
}

void SMTFormula::addTotalizer(const std::vector<literal> &x, std::vector<literal> &y, int K){

	int n = x.size();

	if(n==0){
		y.clear();
		return;
	}
	if(n==1){
		y = x;
		return;
	}

	std::vector<literal> * tree = new std::vector<literal> [2*n-1];

	//Fill tree nodes with coefficients
	for(int i = 0; i < n; i++){
		int idx = n-1+i;
		tree[idx].resize(1);
		tree[idx][0] = x[i];
	}

	for(int i = n-2; i >= 0; i--){
		int ls = tree[lchild(i)].size();
		int rs = tree[rchild(i)].size();
		tree[i].resize(std::min(K+1,ls + rs));
		for(int j = 0; j < std::min(K+1,ls + rs); j++)
			tree[i][j] = newBoolVar();

		for(int j = 0; j < ls; j++){
			addClause(!tree[lchild(i)][j] | tree[i][j]);
			for(int k = 0; k < rs; k++)
				addClause(!tree[lchild(i)][j] | !tree[rchild(i)][k] | tree[i][std::min(K,j+k+1)]);
		}
		for(int k = 0; k < rs; k++)
			addClause(!tree[rchild(i)][k] | tree[i][k]);
	}

	y = tree[0];

	delete [] tree;
}

void SMTFormula::addSorting(const std::vector<literal> &x, std::vector<literal> &y, bool leqclauses, bool geqclauses){
	//Codifies a mergesort
	int n = x.size();

	if(n==0){
		y.clear();
		return;
	}

	if(n==1)
		y=x;
	else if(n==2){
		y.resize(2);
		addTwoComparator(x[0],x[1],y[0],y[1],leqclauses,geqclauses);
	}
	else{
		std::vector<literal> z1,z2;

		std::vector<literal> x1 = std::vector<literal>(x.begin(), x.begin() + n/2);
		std::vector<literal> x2 = std::vector<literal>(x.begin() + n/2, x.end());

		addSorting(x1,z1,leqclauses,geqclauses);
		addSorting(x2,z2,leqclauses,geqclauses);
		addMerge(z1,z2,y,leqclauses,geqclauses);
	}
}


void SMTFormula::addMCardinality(const std::vector<literal> &x, std::vector<literal> &y, int m, bool leqclauses, bool geqclauses){
	//Codifies a mergesort
	int n = x.size();
	

	if(m==0){
		y.clear();
		return;
	}

	if(n<=m){
		addSorting(x,y,leqclauses,geqclauses);
		return;
	}

	std::vector<literal> z1,z2;

	std::vector<literal> x1 = std::vector<literal>(x.begin(), x.begin() + n/2);
	std::vector<literal> x2 = std::vector<literal>(x.begin() + n/2, x.end());

	addMCardinality(x1,z1,std::min(n/2,m),leqclauses,geqclauses);
	addMCardinality(x2,z2,std::min(n-(n/2),m),leqclauses,geqclauses);
	addSimplifiedMerge(z1,z2,y,m,leqclauses,geqclauses);
}

void SMTFormula::copy_to(SMTFormula* f2) const{
    f2->nBoolVars = nBoolVars; f2->nIntVars = nIntVars; f2->nClauses = nClauses;
    f2->hasObjFunc = hasObjFunc; f2->hassoftclauseswithvars = hassoftclauseswithvars; f2->isMinimization = isMinimization;
    f2->LB = LB; f2->UB = UB;
    f2->falsevar = falsevar; f2->truevar = truevar; f2->objFunc = objFunc;

    f2->clauses = vector<clause>(clauses);    f2->softclauses = vector<clause>(softclauses);
    f2->weights = vector<int>(weights);    f2->softclausevars = vector<intvar>(softclausevars);

    f2->mapBoolVars = map<string,boolvar>(mapBoolVars); f2->mapIntVars = map<string,intvar>(mapIntVars);

    f2->boolVarNames = vector<string>(boolVarNames); f2->intVarNames = vector<string>(intVarNames);
    f2->declareVar = vector<bool>(declareVar); //True iff if the i-th int var is a variable has to be declared (i.e. is not a soft clause var)


    f2->defaultauxboolvarpref = defaultauxboolvarpref; f2->defaultauxintvarpref = defaultauxintvarpref;
    f2->auxboolvarpref = auxboolvarpref; f2->auxintvarpref = auxintvarpref;

    f2->use_predef_lits = use_predef_lits; f2->use_predef_order = use_predef_order;
    f2->predef_lits = predef_lits;

}




/*
 MDDBuilder * SMTFormula::addPersistentAMOPB(const std::vector<std::vector<int> > & Q, const std::vector<std::vector<literal> > & X, int K, PBEncoding encoding){
 
 MDDBuilder * mb;
 MDD * mdd;
 std::vector<std::vector<int> > Q2;
 std::vector<std::vector<literal> > X2;
 literal rootvar;
 boolvar undef;
 undef.id=-1;
 switch(encoding){
 case AMOPB_AMOMDD2 :
 Q2 = Q;
 X2 = X;
 util::sortCoefsDecreasing(Q2,X2);
 mb = new AMOPBMDDBuilder(Q2,X2,K);
 
 mdd = mb->getMDD();
 mb->asserted = std::vector<literal>(mdd->getId()+1);
 
 
 for(int i = 0; i <= mdd->getId(); i++)
 mb->asserted[i] = undef;
 rootvar = assertMDDLEQAbio(mdd,mb->asserted);
 addClause(rootvar);
 break;
 
 case AMOPB_AMOMDD :
 mb = new AMOPBMDDBuilder(Q,X,K);
 mdd = mb->getMDD();
 mb->asserted = std::vector<literal>(mdd->getId()+1);
 
 for(int i = 0; i <= mdd->getId(); i++)
 mb->asserted[i] = undef;
 rootvar = assertMDDLEQAbio(mdd,mb->asserted);
 addClause(rootvar);
 
 break;
 
 default:
 std::cerr << "Unsupported encoding for persistent AMOPB" << std::endl;
 exit(SOLVING_ERROR);
 break;
 }
 return mb;
 }
 
 */

/*
 MDDBuilder * SMTFormula::addPersistentAMOPBGT(const std::vector<std::vector<int> > & Q, const std::vector<std::vector<literal> > & X, int K, PBEncoding encoding){
 
 MDDBuilder * mb;
 MDD * mdd;
 std::vector<std::vector<int> > Q2;
 std::vector<std::vector<literal> > X2;
 literal rootvar;
 boolvar undef;
 undef.id=-1;
 switch(encoding){
 case AMOPB_AMOMDD2 :
 Q2 = Q;
 X2 = X;
 util::sortCoefsDecreasing(Q2,X2);
 mb = new AMOPBMDDBuilder(Q2,X2,K-1);
 
 mdd = mb->getMDD();
 
 mb->asserted = std::vector<literal>(mdd->getId()+1);
 for(int i = 0; i <= mdd->getId(); i++)
 mb->asserted[i] = undef;
 
 mb->elses = std::vector<literal>(mdd->getVarDepth());
 for(int i = 0; i < mdd->getVarDepth(); i++)
 mb->elses[i] = undef;
 
 rootvar = assertMDDGTAbio(mdd,mb->asserted,mb->elses);
 addClause(rootvar);
 break;
 
 case AMOPB_AMOMDD :
 mb = new AMOPBMDDBuilder(Q,X,K-1);
 mdd = mb->getMDD();
 
 mb->asserted = std::vector<literal>(mdd->getId()+1);
 for(int i = 0; i <= mdd->getId(); i++)
 mb->asserted[i] = undef;
 
 mb->elses = std::vector<literal>(mdd->getVarDepth());
 for(int i = 0; i < mdd->getVarDepth(); i++)
 mb->elses[i] = undef;
 
 rootvar = assertMDDGTAbio(mdd,mb->asserted,mb->elses);
 addClause(rootvar);
 
 break;
 
 default:
 std::cerr << "Unsupported encoding for persistent AMOPB" << std::endl;
 exit(SOLVING_ERROR);
 break;
 }
 return mb;
 }
 */

/*
 void SMTFormula::addUB(int ub, MDDBuilder * mb){
 MDD * m = mb->addRoot(ub);
 if(m->getId() < mb->asserted.size())
 return;
 
 boolvar undef;
 undef.id=-1;
 
 int s = mb->asserted.size();
 mb->asserted.resize(m->getId()+1);
 
 for(int i = s; i < m->getId()+1; i++)
 mb->asserted[i] = undef;
 
 //sobreescriure antic asserted
 //iniciar els altres a fals
 addClause(assertMDDLEQAbio(m,mb->asserted));
 }
 */

/*
 void SMTFormula::addLB(int lb, MDDBuilder * mb){
 MDD * m = mb->addRoot(lb-1);
 if(m->getId() < mb->asserted.size())
 return;
 
 boolvar undef;
 undef.id=-1;
 
 int s = mb->asserted.size();
 mb->asserted.resize(m->getId()+1);
 
 for(int i = s; i < m->getId()+1; i++)
 mb->asserted[i] = undef;
 
 //sobreescriure antic asserted
 //iniciar els altres a fals
 addClause(assertMDDGTAbio(m,mb->asserted,mb->elses));
 }
 */



}

