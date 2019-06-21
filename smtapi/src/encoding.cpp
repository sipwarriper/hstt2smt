#include "encoding.h"
#include "errors.h"
#include <iostream>

using namespace std;

Encoding::Encoding(){

}

Encoding::~Encoding(){

}

void Encoding::setModel(const EncodedFormula & ef, int lb, int ub, const vector<bool> & bmodel, const vector<int> & imodel){

}

int Encoding::getObjective() const{
	return INT_MIN;
}

void Encoding::assumeBounds(const EncodedFormula & ef, int LB, int UB, vector<literal> & assumptions){
	cerr << "Error: tried to solve using assumptions, but the selected encoding does not implement checks with assumptions." << endl;
	exit(SOLVING_ERROR);
}

bool Encoding::narrowBounds(const EncodedFormula & ef, int lastLB, int lastUB, int lb, int ub){
	return false;
}

bool Encoding::printSolution(ostream & os) const{
	return false;
}


