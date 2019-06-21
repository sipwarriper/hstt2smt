#ifndef AMOPBBDDBUILDER_DEF
#define AMOPBBDDBUILDER_DEF

#include <iostream>
#include <cstdio>
#include <vector>
#include <map>
#include <list>
#include <climits>
#include <algorithm>
#include "mddbuilder.h"
#include "mdd.h"

using namespace std;

struct R_B;

class AMOPBBDDBuilder : public MDDBuilder {

private:
	vector<vector<MDD *> > L_BDDs;
	vector<vector<R_B> > L;

	int K;
	vector<vector<int> > Q;
	vector<vector<literal> > X;

	void initL();
	void mostrarL(int i_l=0);
	void insertBDD(R_B rb_in,int i_l);
	int inf_sum(int possible_inf, int x);

	R_B searchBDD(int i_k,int i_l);

	R_B BDDConstruction(int globa_l, int local_l, int l, int i_k);
	int getmax(const vector<int> & v);
	int getmax(const vector<int> & v, int init);

	bool hasInconsistentPath(literal v1, literal v2) const;

	void createGraphviz(MDD * mdd, ostream & os, bool * visited,int * global_idx,int * local_idx) const;


protected:
	virtual MDD * buildMDD();

public:

	//Constructor
	AMOPBBDDBuilder(const vector<vector<int> > &Q,const vector<vector<literal> > &X, int K, bool longedges=true);
	AMOPBBDDBuilder(const vector<int> &Q,const vector<literal> &X, int K, bool longedges=true);
	~AMOPBBDDBuilder();


	virtual void createGraphviz(ostream & os, vector<vector<int> > * labels = NULL) const;


	bool hasInconsistentPath(const list<pair<literal,literal> > & mutexes) const;

};


//Encapsulation of an MDD with its interval [B,Y]
struct R_B {
MDD *bdd;
int B;
int Y;

R_B() {
    bdd=NULL;
    B=0;
    Y=0;
}
R_B(MDD * m, int b, int y) {
    bdd=m;
    B=b;
    Y=y;
}
};


#endif
