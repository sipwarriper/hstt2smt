#include "auctionencoding.h"
#include "util.h"
#include "errors.h"
#include <limits.h>

AuctionEncoding::AuctionEncoding(Auction * instance, AMOPBEncoding amopbenc) : Encoding()  {
	this->instance = instance;
	this->amopbenc = amopbenc;
}

AuctionEncoding::~AuctionEncoding() {

}

SMTFormula * AuctionEncoding::encode(int lb, int ub){

	SMTFormula * f = new SMTFormula();

	vector<literal> x(instance->getNBids());

    for(int i = 0; i < instance->getNBids(); i++)
		x[i]=f->newBoolVar("x",i);

    for(int i = 0; i < instance->getNBids()-1; i++)
		for(int item : instance->getBid(i))
			for(int j = i+1; j < instance->getNBids(); j++)
				if(instance->demandsItem(j,item))
					f->addClause(!x[i] | !x[j]);

	/*
	boolvar v; // variable booleana
	literal l = v; // literal
	l = !v;

	clause c;
	c = v;
	c = l;
	c = !v;
	c = v | !v2 | l;

	f->addAMO(x);
	f->addEO(x);
	f->addAMK(x,3);
	f->addPB(q,x,7);



	intvar i = f->newIntVar("inad",1,3,5);
	intvar i2 = f->newIntVar("inad",1,2,5);

	intsum;
	for(int a = 0; a < 10; a++)
		intsum+= 3*f->ivar("inad",a,1,1);

	literal l = intsum <= 10;

	x1 - x2 <= 5;
*/

	vector<vector<int> > cover;
	instance->computeBidCover(cover);

	vector<vector<literal> > X(cover.size());
	vector<vector<int> > Q(cover.size());

	for(int i = 0; i < cover.size(); i++){
		for(int j : cover[i]){
			Q[i].push_back(instance->getBidValue(j));
			X[i].push_back(x[j]);
		}
	}


	f->addAMOPBGEQ(Q,X,lb,amopbenc);

	return f;
}


void AuctionEncoding::setModel(const EncodedFormula & ef, int lb, int ub, const vector<bool> & bmodel, const vector<int> & imodel){

	this->sold.resize(instance->getNBids());
	for (int i=0; i<instance->getNBids(); i++)
		sold[i] = SMTFormula::getBValue(ef.f->bvar("x",i),bmodel);
}

int AuctionEncoding::getObjective() const{
	int sum = 0;
	for (int i=0; i<instance->getNBids(); i++)
		if(sold[i])
			sum+=instance->getBidValue(i);

	return sum;
}


bool AuctionEncoding::printSolution(ostream & os) const {
	for(int i = 0; i < instance->getNBids(); i++)
		if(sold[i])
			os << " :" << i;
	os << endl;
	return true;
}
