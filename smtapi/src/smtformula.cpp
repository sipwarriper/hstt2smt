#include "smtformula.h"
#include "amopbmddbuilder.h"
#include "amopbbddbuilder.h"
#include "mdd.h"
#include "errors.h"
#include "util.h"
#include <limits.h>
#include <algorithm>
#include <math.h>

using namespace std;

namespace smtapi{


map<AMOPBEncoding,PBEncoding> amopb_pb_rel = {
	{AMOPB_BDD,PB_BDD},
	{AMOPB_BDD2,PB_BDD2},
	{AMOPB_SWC,PB_SWC},
	{AMOPB_GT,PB_GT},
	{AMOPB_GT2,PB_GT2},
	{AMOPB_GPW,PB_GPW},
	{AMOPB_LPW,PB_LPW},
	{AMOPB_GBM,PB_GBM},
	{AMOPB_LBM,PB_LBM}
};


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

const vector<clause> & SMTFormula::getClauses() const{
	return clauses;
}

const vector<clause> & SMTFormula::getSoftClauses() const{
	return softclauses;
}

const vector<int> & SMTFormula::getWeights() const{
	return weights;
}

const vector<intvar> & SMTFormula::getSoftClauseVars() const{
	return softclausevars;
}

int SMTFormula::getHardWeight() const{
	int sum = 2;
	for(int i : weights)
		sum+= i;
	return sum;
}

const vector<string> & SMTFormula::getBoolVarNames() const{
	return boolVarNames;
}

const vector<string> & SMTFormula::getIntVarNames() const{
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
	boolVarNames.push_back("a__"+to_string(x.id));
	return x;
}

boolvar SMTFormula::newBoolVar(const string & s){
	boolvar x;
	x.id=++nBoolVars;
	mapBoolVars[s] = x;
	boolVarNames.push_back(s);
	return x;
}

boolvar SMTFormula::newBoolVar(const string & s, int i1){
	boolvar x;
	x.id=++nBoolVars;
	string s2 = ssubs(s,i1);
	mapBoolVars[s2] = x;
	boolVarNames.push_back(s2);
	return x;
}

boolvar SMTFormula::newBoolVar(const string & s, int i1, int i2){
	boolvar x;
	x.id=++nBoolVars;
	string s2 = ssubs(s,i1,i2);
	mapBoolVars[s2] = x;
	boolVarNames.push_back(s2);
	return x;
}

boolvar SMTFormula::newBoolVar(const string & s, int i1, int i2, int i3){
	boolvar x;
	x.id=++nBoolVars;
	string s2 = ssubs(s,i1,i2,i3);
	mapBoolVars[s2] = x;
	boolVarNames.push_back(s2);
	return x;
}

intvar SMTFormula::newIntVar(bool declare){
	intvar x;
	x.id=++nIntVars;
	intVarNames.push_back("a__"+to_string(x.id));
	declareVar.push_back(declare);
	return x;
}

intvar SMTFormula::newIntVar(const string & s, bool declare){
	intvar x;
	x.id=++nIntVars;
	mapIntVars[s] = x;
	intVarNames.push_back(s);
	declareVar.push_back(declare);
	return x;
}

intvar SMTFormula::newIntVar(const string & s, int i1, bool declare){
	intvar x;
	x.id=++nIntVars;
	string s2 = ssubs(s,i1);
	mapIntVars[s2] = x;
	intVarNames.push_back(s2);
	declareVar.push_back(declare);
	return x;
}

intvar SMTFormula::newIntVar(const string & s, int i1, int i2, bool declare){
	intvar x;
	x.id=++nIntVars;
	string s2 = ssubs(s,i1,i2);
	mapIntVars[s2] = x;
	intVarNames.push_back(s2);
	declareVar.push_back(declare);
	return x;
}

intvar SMTFormula::newIntVar(const string & s, int i1, int i2, int i3, bool declare){
	intvar x;
	x.id=++nIntVars;
	string s2 = ssubs(s,i1,i2,i3);
	mapIntVars[s2] = x;
	intVarNames.push_back(s2);
	declareVar.push_back(declare);
	return x;
}

boolvar SMTFormula::bvar(const string & s) const{
	return mapBoolVars.find(s)->second;
}

boolvar SMTFormula::bvar(const string & s, int i1) const{
	return mapBoolVars.find(ssubs(s,i1))->second;
}

boolvar SMTFormula::bvar(const string & s, int i1, int i2) const{
	return mapBoolVars.find(ssubs(s,i1,i2))->second;
}

boolvar SMTFormula::bvar(const string & s, int i1, int i2, int i3) const{
	return mapBoolVars.find(ssubs(s,i1,i2,i3))->second;
}

intvar SMTFormula::ivar(const string & s) const{
	return mapIntVars.find(s)->second;
}

intvar SMTFormula::ivar(const string & s, int i1) const{
	return mapIntVars.find(ssubs(s,i1))->second;
}

intvar SMTFormula::ivar(const string & s, int i1, int i2) const{
	return mapIntVars.find(ssubs(s,i1,i2))->second;
}

intvar SMTFormula::ivar(const string & s, int i1, int i2, int i3) const{
	return mapIntVars.find(ssubs(s,i1,i2,i3))->second;
}


string SMTFormula::ssubs(const string & s, int i1) const{
	char aux[50];
	sprintf(aux,"%s_%d",s.c_str(),i1);
	return aux;
}

string SMTFormula::ssubs(const string & s, int i1, int i2) const{
	char aux[50];
	sprintf(aux,"%s_%d_%d",s.c_str(),i1,i2);
	return aux;
}

string SMTFormula::ssubs(const string & s, int i1, int i2, int i3) const{
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

int SMTFormula::getIValue(const intvar & var, const vector<int> & vals){
	return vals[var.id];
}

bool SMTFormula::getBValue(const boolvar & var, const vector<bool> & vals){
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

void SMTFormula::addClauses(const vector<clause> &c) {
	clauses.insert(clauses.end(),c.begin(),c.end());
}

void SMTFormula::addALO(const vector<literal> & v) {
	addClause(v);
}

void SMTFormula::addAMO(const vector<literal> & x, AMOEncoding enc) {

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
			vector<boolvar> y(m);
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
			vector<boolvar> y(n-1);
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
				vector<literal> v1(x.begin(),x.begin()+2);
				vector<literal> v2(x.begin()+2,x.end());
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

				vector<literal> cmd_vars(nsplits);

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
					vector<literal> v;
					for(int j = 3*i; j < 3*(i+1) && j < n; j++){
						v.push_back(x[j]);
						addClause(v[i] | !x[j]);
					}
					addAMO(v,AMO_QUAD);
				}
			}
		}
		default:
			cerr << "Wrong AMO encoding" << endl;
			exit(BADCODIFICATION_ERROR);
			break;
	}
}

void SMTFormula::addEO(const vector<literal> & v, AMOEncoding enc) {
	addALO(v);
	addAMO(v,enc);
}


void SMTFormula::addALK(const vector<literal> & v, int K){
	int n = v.size();
	if(K>n){ //Trivially false
		addEmptyClause();
		return;
	}
	else if(K<0)//Trivially true
		return;
	else if(n==0 && K == 0)//Trivially true
		return;
	else if(n==1 && K==1)
		addClause(v[0]);
	else{
		vector<literal> sorted(n);
		addSorting(v,sorted);
		if(K>0) addClause(sorted[K-1]);
	}
}

void SMTFormula::addAMK(const vector<literal> & v, int K, CardinalityEncoding enc){
	int n = v.size();
	if(K < 0){ //Trivially false
		addEmptyClause();
		return;
	}
	else if(K>n){ //Trivially true
		return;
	}
	else if((n==0 && K == 0) || (n==1 && K==1)) //Trivially true
		return;
	else{
		switch(enc){
			case CARD_TOTALIZER:
			{
				vector<literal> root(min(K+1,n));
				addTotalizer(v,root,K);
				if(root.size() > K)
					addClause(!root[K]);
			}
			break;

			case CARD_SORTER:
			default:
			{
				vector<literal> sorted(n);
				addSorting(v,sorted);
				if(K<n)
					addClause(!sorted[K]);
			}
			break;
		}
	}
}

void SMTFormula::addEK(const vector<literal> & v, int K){
	int n = v.size();
	if(K>n | K < 0){ //Triviavlly false
		addEmptyClause();
		return;
	}
	else if(n==0 && K == 0) //Trivially true
		return;
	else if(n==1 && K==1)
		addClause(v[0]);
	else{
		vector<literal> sorted(n);
		addSorting(v,sorted);
		if(K>0) addClause(sorted[K-1]);
        if(K<n) addClause(!sorted[K]);
	}
}

void SMTFormula::addPB(const vector<int> & Q, const vector<literal> & X, int K, PBEncoding encoding){

	vector<vector<int> > QQ;
	vector<vector<literal> > XX;
	switch(encoding){
		case PB_BDD:
		{
			AMOPBMDDBuilder mb(Q,X,K);
			MDD * mdd = mb.getMDD();
			addClause(assertMDDLEQAbio(mdd));
		}
			break;

		case PB_BDD2:
		{
			vector<int> Q2 = Q;
			vector<literal> X2 = X;
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

		case PB_GT2:
			util::PBtoAMOPB(Q,X,QQ,XX);
			addAMOPB(QQ,XX,K,AMOPB_GT2);
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
			cerr << "Wrong kind of PB encoding" << endl;
			exit(BADCODIFICATION_ERROR);
			break;
	}
}

void SMTFormula::addAMOPB(const vector<vector<int> > & Q, const vector<vector<literal> > & X, int K, AMOPBEncoding encoding){

	map<AMOPBEncoding,PBEncoding>::iterator it = amopb_pb_rel.find(encoding);
	if(it != amopb_pb_rel.end()){
		vector<int> Q2;
		vector<literal> X2;
		util::AMOPBtoPB(Q,X,Q2,X2);
		addPB(Q2,X2,K,it->second);
		return;
	}


	if(X.size()!=Q.size())
	{
		cerr << "Tried to assert an AMO-PB with different number of coefficients and variables" << endl;
		exit(BADCODIFICATION_ERROR);
	}

	vector<vector<int> > Q2;
	vector<vector<literal> > X2;

	int maxsum = 0;

	for(int i = 0; i < X.size(); i++){
		if(X[i].size()!=Q[i].size()){
			cerr << "Tried to assert an AMO-PB with different number of coefficients and variables" << endl;
			exit(BADCODIFICATION_ERROR);
		}

		int max = 0;
		vector<int> q;
		vector<literal> x;
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
		for(const vector<literal> & v : X2)
			for(const literal & l : v)
				addClause(!l);
		return;
	}
	if(K >= maxsum)
		return;

	int N = X2.size();
	if(N==1) //We have enforced that no AMO product contains a coefficient greater than K
		return;

	switch(encoding){
		case AMOPB_AMOMDD2 :
			util::sortCoefsDecreasing(Q2,X2);

		case AMOPB_AMOMDD :
		{
			AMOPBMDDBuilder mb(Q2,X2,K);
			MDD * mdd = mb.getMDD();
			addClause(assertMDDLEQAbio(mdd));
		}
			break;

		case AMOPB_IMPCHAIN :
		{
			vector<vector<literal> > Y2;
			for(int i = 0; i < X2.size();i++){
				Y2.push_back(vector<literal>(X2[i].size()));
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

		case AMOPB_GT2:
			addAMOPBGeneralizedTotalizer2(Q2,X2,K);
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
			cerr << "Wrong kind of AMOPB encoding" << endl;
			exit(SOLVING_ERROR);
			break;
	}
}

/*
MDDBuilder * SMTFormula::addPersistentAMOPB(const vector<vector<int> > & Q, const vector<vector<literal> > & X, int K, PBEncoding encoding){

	MDDBuilder * mb;
	MDD * mdd;
	vector<vector<int> > Q2;
	vector<vector<literal> > X2;
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
			mb->asserted = vector<literal>(mdd->getId()+1);


			for(int i = 0; i <= mdd->getId(); i++)
				mb->asserted[i] = undef;
			rootvar = assertMDDLEQAbio(mdd,mb->asserted);
			addClause(rootvar);
			break;

		case AMOPB_AMOMDD :
			mb = new AMOPBMDDBuilder(Q,X,K);
			mdd = mb->getMDD();
			mb->asserted = vector<literal>(mdd->getId()+1);

			for(int i = 0; i <= mdd->getId(); i++)
				mb->asserted[i] = undef;
			rootvar = assertMDDLEQAbio(mdd,mb->asserted);
			addClause(rootvar);

			break;

		default:
			cerr << "Unsupported encoding for persistent AMOPB" << endl;
			exit(SOLVING_ERROR);
			break;
	}
	return mb;
}

*/


void SMTFormula::addPBGEQ(const vector<int> & Q, const vector<literal> & X, int K, PBEncoding encoding){

	vector<literal> X2;
	int K2=0;

	for(int i = 0; i < Q.size(); i++){
		X2[i]=!X[i];
		K2+=Q[i];
	}
	K2 -= K;

	addPB(Q,X2,K,encoding);
}


void SMTFormula::addAMOPBGEQ(const vector<vector<int> > & Q, const vector<vector<literal> > & X, int K, AMOPBEncoding encoding){

	vector<vector<int> > Q2(Q.size());
	vector<vector<literal> > X2(X.size());
	int K2=0;

	for(int i = 0; i < Q.size(); i++){
		if(Q[i].size()==1){
			X2[i].push_back(!X[i][0]);
			Q2[i].push_back(Q[i][0]);
			K2+=Q[i][0];
		}
		else{
			boolvar aux = newBoolVar();
			int maxq = *max_element(Q[i].begin(),Q[i].end());
			vector<literal> c;
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

	K2 -= K;

	addAMOPB(Q2,X2,K2,encoding);
}

/*
MDDBuilder * SMTFormula::addPersistentAMOPBGT(const vector<vector<int> > & Q, const vector<vector<literal> > & X, int K, PBEncoding encoding){

	MDDBuilder * mb;
	MDD * mdd;
	vector<vector<int> > Q2;
	vector<vector<literal> > X2;
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

			mb->asserted = vector<literal>(mdd->getId()+1);
			for(int i = 0; i <= mdd->getId(); i++)
				mb->asserted[i] = undef;

			mb->elses = vector<literal>(mdd->getVarDepth());
			for(int i = 0; i < mdd->getVarDepth(); i++)
				mb->elses[i] = undef;

			rootvar = assertMDDGTAbio(mdd,mb->asserted,mb->elses);
			addClause(rootvar);
			break;

		case AMOPB_AMOMDD :
			mb = new AMOPBMDDBuilder(Q,X,K-1);
			mdd = mb->getMDD();

			mb->asserted = vector<literal>(mdd->getId()+1);
			for(int i = 0; i <= mdd->getId(); i++)
				mb->asserted[i] = undef;

			mb->elses = vector<literal>(mdd->getVarDepth());
			for(int i = 0; i < mdd->getVarDepth(); i++)
				mb->elses[i] = undef;

			rootvar = assertMDDGTAbio(mdd,mb->asserted,mb->elses);
			addClause(rootvar);

			break;

		default:
			cerr << "Unsupported encoding for persistent AMOPB" << endl;
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

literal SMTFormula::assertMDDLEQAbio(MDD * mdd) {
	vector<literal> asserted(mdd->getId()+1);
	boolvar undef;
	undef.id=-1;

	for(int i = 0; i <= mdd->getId(); i++)
		asserted[i] = undef;
	literal rootvar = assertMDDLEQAbio(mdd,asserted);
	return rootvar;
}

literal SMTFormula::assertMDDLEQAbio(MDD * mdd, vector<literal> & asserted) {
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
			pair<literal,MDD *> p = mdd->getSelectorAndChild(i);
			if(p.second != mdd->getElseChild())
			{
				literal vi = assertMDDLEQAbio(p.second,asserted);
				addClause(vi | !p.first | !v);
			}
		}
	}

	return v;
}

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

/*
literal SMTFormula::assertMDDGTAbio(MDD * mdd) {
	vector<literal> asserted(mdd->getId()+1);
	boolvar undef;
	undef.id=-1;

	vector<literal>  elses(mdd->getVarDepth());
	for(int i = 0; i < mdd->getVarDepth(); i++)
		elses[i] = undef;

	for(int i = 0; i <= mdd->getId(); i++)
		asserted[i] = undef;
	literal rootvar = assertMDDGTAbio(mdd,asserted, elses);
	return rootvar;
}

literal SMTFormula::assertMDDGTAbio(MDD * mdd, vector<literal> & asserted, vector<literal> & elses) {
	if(mdd->isTrueMDD())
		return falseVar();
	else if(mdd->isFalseMDD())
		return trueVar();

	literal v=asserted[mdd->getId()];
	if(v.v.id==-1){
		v = newBoolVar();
		asserted[mdd->getId()]=v;

		if(elses[mdd->getVarDepth()-1].v.id == -1){
			if(mdd->getNSelectors()==1)
				elses[mdd->getVarDepth()-1] = !mdd->getSelectors()[0];
			else{
				elses[mdd->getVarDepth()-1] = newBoolVar();
				clause c = elses[mdd->getVarDepth()-1];
				for(const literal & l : mdd->getSelectors())
					c |= l;
				addClause(c);
			}
		}

		literal velse = assertMDDGTAbio(mdd->getElseChild(),asserted,elses);
		addClause(velse | !elses[mdd->getVarDepth()-1] | !v);

		for(int i = 1; i < mdd->getNSelectors();i++){
			pair<literal,MDD *> p = mdd->getSelectorAndChild(i);
			if(p.second != mdd->getChildByIdx(0))
			{
				literal vi = assertMDDGTAbio(p.second,asserted,elses);
				addClause(vi | !p.first | !v);
			}
		}
		literal vmax = assertMDDGTAbio(mdd->getChildByIdx(0),asserted,elses);
		addClause(vmax | !v);
	}

	return v;
}
*/

void SMTFormula::addAMOPBSWC(const vector<vector<int> > & Q, const vector<vector<literal> > & X, int K){

	int N = X.size();

	vector<literal> Sin, Sout(K);

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

void SMTFormula::addAMOPBSorter(const vector<vector<int> > & Q, const vector<vector<literal> > & X, int K){

	int n = Q.size();

	if(K==0){
		for(const vector<literal> & v : X)
			for(const literal & l : v)
				addClause(!l);
		return;
	}

	int maxsum = 0;
	for(int i = 0; i < n; i++)
		maxsum+= *(max_element(Q[i].begin(),Q[i].end()));

	if(maxsum <= K)
		return;

	vector<vector<literal> > orders;
	for(int i = 0; i < n; i++){
		int maxq = *(max_element(Q[i].begin(),Q[i].end()));
		if(maxq > 0){
			vector<literal> order;
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
		vector<literal> result;
		addSimplifiedMerge(orders[0],orders[1],result,K+1);
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

void SMTFormula::addAMOPBGeneralizedTotalizer(const vector<vector<int> > & Q, const vector<vector<literal> > & X, int K){

	int n = X.size();

	vector<int> * tree = new vector<int>[2*n-1];
	vector<literal> * treevars = new vector<literal> [2*n-1];
	map<int,literal> * treevarsmap = new map<int,literal> [n-1];

	//Fill tree nodes with coefficients
	for(int i = 0; i < n; i++){
		int idx = n-1+i;
		tree[idx].clear();
		treevars[idx].clear();
		map<int,int> count;
		map<int,literal> lit;
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
		for(const pair<int,literal> & p : lit){
			tree[idx].push_back(p.first);
			treevars[idx].push_back(p.second);
		}
		tree[idx].push_back(0);
	}

	for(int i = n-2; i >= 0; i--){
		tree[i].clear();
		for(int j = 0; j < tree[lchild(i)].size(); j++){
			for(int k = 0; k < tree[rchild(i)].size(); k++){
				int x = min(tree[lchild(i)][j]+tree[rchild(i)][k],K+1);
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
				int x = min(tree[l][j]+tree[r][k],K+1);
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

void SMTFormula::addAMOPBGeneralizedTotalizer2(const vector<vector<int> > & Q, const vector<vector<literal> > & X, int K){

	if(K==0){
		for(const vector<literal> & v : X)
			for(const literal & l : v)
				addClause(!l);
		return;
	}
	else if(K < 0){
		addEmptyClause();
		return;
	}
	int n = Q.size();

	if(n==1){
		for(int i = 0; i < Q[0].size(); i++)
			if(Q[0][i] > K)
				addClause(!X[0][i]);
		return;
	}

	vector<int> * tree = new vector<int>[2*n-1];
	vector<literal> * treevars = new vector<literal> [2*n-1];
	map<int,literal> * treevarsmap = new map<int,literal> [n-1];

	//Fill leaf nodes with coefficients
	for(int i = 0; i < n; i++){
		int idx = n-1+i;
		tree[idx].clear();
		treevars[idx].clear();
		map<int,int> count;
		map<int,literal> lit;
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
		for(const pair<int,literal> & p : lit){
			tree[idx].push_back(p.first);
			treevars[idx].push_back(p.second);
		}
		tree[idx].push_back(0);
	}

	for(int i = n-2; i >= 0; i--){
		tree[i].clear();
		for(int j = 0; j < tree[lchild(i)].size(); j++){
			for(int k = 0; k < tree[rchild(i)].size(); k++){
				int x = tree[lchild(i)][j]+tree[rchild(i)][k];
				if(x <= K && !util::existsSorted(tree[i],x))
					util::insertSorted(tree[i],x);
			}
		}
	}

//	for(int i = 0; i < 2*n-1; i++){
//		cout << tree[i].size() << " : <";
//		for(int x : tree[i])
//			cout << x << " ";
//		cout << ">" << endl;
//		if(util::isPowerOf2(i+2)) cout << endl << endl;
//	}

	//Encode the tree
	for(int i = n-2; i >= 0; i--){
		if(i > 0){
			for(int j = 0; j < tree[i].size()-1; j++){
				boolvar v = newBoolVar();
				treevars[i].push_back(v);
				treevarsmap[i][tree[i][j]] = v;
			}
		}

		int l = lchild(i);
		int r = rchild(i);

		for(int j = 0; j < tree[l].size(); j++){
			for(int k = 0; k < tree[r].size(); k++){
				int x = tree[l][j]+tree[r][k];
				if(x > K){
					if(j == tree[l].size()-1)
						addClause(!treevars[r][k]);
					else if(k == tree[r].size()-1)
						addClause(!treevars[l][j]);
					else
						addClause(!treevars[l][j] | !treevars[r][k]);
				}
				else if(i > 0 && treevarsmap[i].find(x) != treevarsmap[i].end()){
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

	delete [] tree;
	delete [] treevars;
	delete [] treevarsmap;
}



literal SMTFormula::addPolynomialWatchdog(const vector<vector<int> > & Q, const vector<vector<literal> > & X, int K, bool useSorter){
	int n = Q.size();

	//This encoding is for < K instead of <= K
	K+=1;

	int max = 0;
	for(const vector<int> & q : Q)
		for(int qi : q)
			if(qi > max)
				max = qi;

	int p = (int)floor(log2(max));
	int p2 = (int) exp2(p);
	int m = K / p2;
	if(K%p2 != 0)
		m++;
	int T = (m*p2) - K;

	vector<vector<literal> > B(p+1); //Buckets

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

	vector<literal> S, Shalf;
	for(int i = 0; i <= p; i++){
		S.clear();
		vector<literal> U;
		if(useSorter)
			addSorting(B[i],U);
		else
			addTotalizer(B[i],U);
		if(i==0)
			S=U;
		else{
			if(useSorter)
				addMerge(U,Shalf,S);
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


void SMTFormula::addAMOPBGlobalPolynomialWatchdog(const vector<vector<int> > & Q, const vector<vector<literal> > & X, int K, bool useSorter){
	if(K==0){
		for(const vector<literal> & v : X)
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

void SMTFormula::addAMOPBLocalPolynomialWatchdog(const vector<vector<int> > & Q, const vector<vector<literal> > & X, int K, bool useSorter){
	if(K==0){
		for(const vector<literal> & v : X)
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
		vector<vector<int> > Q2 = Q;
		vector<vector<literal> > X2 = X;
		Q2[i]=Q2[Q2.size()-1];
		Q2.resize(Q2.size()-1);
		X2[i]=X2[X2.size()-1];
		X2.resize(X2.size()-1);
		for(int j = 0; j < Q[i].size(); j++){
			map<int,literal> watchdog;
			if(watchdog.find(Q[i][j]) == watchdog.end())
				watchdog[Q[i][j]] = addPolynomialWatchdog(Q2,X2,K-Q[i][j],useSorter);
			addClause(!watchdog[Q[i][j]]| !X[i][j]);
		}
	}
}

void SMTFormula::addOrderEncoding(int x, vector<literal> & lits){

	lits.resize(x);
	lits[0]=newBoolVar();
	for(int i = 1; i < x; i++){
		lits[i] = newBoolVar();
		addClause(!lits[i] | lits[i-1]);
	}
}



void SMTFormula::addTwoComparator(const literal &x1, const literal &x2, literal &y1, literal &y2){
	y1 = newBoolVar();
	y2 = newBoolVar();

	addClause(!x1 | y1);
	addClause(!x2 | y1);
	addClause(!x1 | ! x2 | y2);

	addClause(x1 | !y2);
	addClause(x2 | !y2);
	addClause(x1 | x2 | !y1);
}

void SMTFormula::addQuadraticMerge(const vector<literal> &x1, const vector<literal> &x2, vector<literal> &y){
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

void SMTFormula::addMerge(const vector<literal> &x1, const vector<literal> &x2, vector<literal> &y){
	int a = x1.size();
	int b = x2.size();

	if(a==0 && b==0){
		y.clear();
		return;
	}

	y.resize(a+b);

	if(a==1 && b==1)
		addTwoComparator(x1[0],x2[0],y[0],y[1]);
	else if(a == 0)
		y = x2;
	else if(b == 0)
		y = x1;
	else{
		vector<literal> x1even, x1odd, x2even, x2odd;
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

		vector<literal> zeven;
		vector<literal> zodd;

		addMerge(x1even, x2even, zeven);
		addMerge(x1odd, x2odd,zodd);

		vector<literal> z(a+b);
		if(a%2==0){
			if(b%2==0){
				for(int i = 0; i < (a+b)/2; i++)
					z[2*i] = zeven[i];

				for(int i = 0; i < (a+b)/2; i++)
					z[2*i + 1] = zodd[i];

				y[0] = z[0];
				y[a+b-1] = z[a+b-1];
				for(int i = 1; i < a+b-2; i+=2)
					addTwoComparator(z[i],z[i+1],y[i],y[i+1]);

			}else{
				for(int i = 0; i < (a+b)/2 + 1; i++)
					z[2*i] = zeven[i];

				for(int i = 0; i < (a+b)/2; i++)
					z[2*i + 1] = zodd[i];

				y[0] = z[0];
				for(int i = 1; i < a+b-1; i+=2)
					addTwoComparator(z[i],z[i+1],y[i],y[i+1]);

			}
		}
		else{ //a%2==1
			if(b%2==0)
				addMerge(x2,x1,y);
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
					addTwoComparator(z[i],z[i+1],y[i],y[i+1]);

			}
		}
	}
}


void SMTFormula::addSimplifiedMerge(const vector<literal> &x1, const vector<literal> &x2, vector<literal> &y, int c){
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
		addClause(!x1[0] | y[0]);
		addClause(!x2[0] | y[0]);
	}
	else if(a > c)
		addSimplifiedMerge(vector<literal>(x1.begin(),x1.begin()+c),x2,y,c);
	else if(b > c)
		addSimplifiedMerge(x1,vector<literal>(x2.begin(),x2.begin()+c),y,c);
	else if(a+b<=c)
		addMerge(x1,x2,y);
	else{
		y.resize(c);
		vector<literal> x1even, x1odd, x2even, x2odd;
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

		vector<literal> zeven;
		vector<literal> zodd;
		vector<literal> z;

		if(c%2==0){
			addSimplifiedMerge(x1even, x2even, zeven, c/2 + 1);
			addSimplifiedMerge(x1odd, x2odd, zodd, c/2);

			z.resize(c+1);
			for(int i = 0; i < c/2; i++){
				z[2*i] = zeven[i];
				z[2*i +1] = zodd[i];
			}
			z[c] = zeven[c/2];

			y[0] = z[0];
			for(int i = 1; i < c-2; i+=2)
				addTwoComparator(z[i],z[i+1],y[i],y[i+1]);
			y[c-1]=newBoolVar();
			addClause(!z[c-1] | y[c-1]);
			addClause(!z[c] | y[c-1]);
		}
		else{ //c%2==1
			addSimplifiedMerge(x1even, x2even, zeven, (c+1)/2);
			addSimplifiedMerge(x1odd, x2odd, zodd, (c-1)/2);

			z.resize(c);
			for(int i = 0; i < (c-1)/2; i++){
				z[2*i] = zeven[i];
				z[2*i +1] = zodd[i];
			}
			z[c-1] = zeven[(c-1)/2];

			y[0] = z[0];
			for(int i = 1; i < c-1; i+=2)
				addTwoComparator(z[i],z[i+1],y[i],y[i+1]);
		}
	}
}

void SMTFormula::addTotalizer(const vector<literal> &x, vector<literal> &y){

	int n = x.size();

	if(n==0){
		y.clear();
		return;
	}
	if(n==1){
		y = x;
		return;
	}

	vector<literal> * tree = new vector<literal> [2*n-1];

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

void SMTFormula::addTotalizer(const vector<literal> &x, vector<literal> &y, int K){

	int n = x.size();

	if(n==0){
		y.clear();
		return;
	}
	if(n==1){
		y = x;
		return;
	}

	vector<literal> * tree = new vector<literal> [2*n-1];

	//Fill tree nodes with coefficients
	for(int i = 0; i < n; i++){
		int idx = n-1+i;
		tree[idx].resize(1);
		tree[idx][0] = x[i];
	}

	for(int i = n-2; i >= 0; i--){
		int ls = tree[lchild(i)].size();
		int rs = tree[rchild(i)].size();
		tree[i].resize(min(K+1,ls + rs));
		for(int j = 0; j < min(K+1,ls + rs); j++)
			tree[i][j] = newBoolVar();

		for(int j = 0; j < ls; j++){
			addClause(!tree[lchild(i)][j] | tree[i][j]);
			for(int k = 0; k < rs; k++)
				addClause(!tree[lchild(i)][j] | !tree[rchild(i)][k] | tree[i][min(K,j+k+1)]);
		}
		for(int k = 0; k < rs; k++)
			addClause(!tree[rchild(i)][k] | tree[i][k]);
	}

	y = tree[0];

	delete [] tree;
}

void SMTFormula::addSorting(const vector<literal> &x, vector<literal> &y){
	//Codifies a mergesort
	int n = x.size();

	if(n==0){
		y.clear();
		return;
	}

	y.resize(n);
	if(n==1)
		y=x;
	else if(n==2)
		addTwoComparator(x[0],x[1],y[0],y[1]);
	else{
		vector<literal> z1,z2;

		vector<literal> x1 = vector<literal>(x.begin(), x.begin() + n/2);
		vector<literal> x2 = vector<literal>(x.begin() + n/2, x.end());

		addSorting(x1,z1);
		addSorting(x2,z2);
		addSimplifiedMerge(z1,z2,y,z1.size()+z2.size());
	}
}

}

