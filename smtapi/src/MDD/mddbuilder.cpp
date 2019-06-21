//Proces de creacio i assercio del constraint

#include "mddbuilder.h"

using namespace std;


MDDBuilder::MDDBuilder() {
  nodeCount = 2;
  this->root = NULL;
}

MDDBuilder::~MDDBuilder(){
}

MDD * MDDBuilder::getMDD(){
	if(root == NULL)
		root = buildMDD();
   return root;
}

MDD * MDDBuilder::addRoot(int k){
	cerr << "Shared MDD not implemented for selected kind of MDD" << endl;
	exit(UNSUPPORTEDFUNC_ERROR);
}

int MDDBuilder::getSize() const{
	return nodeCount == 2 ? 1 : nodeCount;
}


