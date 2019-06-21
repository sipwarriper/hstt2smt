#ifndef MDDBUILDERDEF
#define MDDBUILDERDEF

#include <iostream>
#include <cstdio>
#include <vector>
#include <map>
#include <list>
#include <climits>
#include <algorithm>
#include "errors.h"
#include "mdd.h"

using namespace std;


/*
 * This is a pure virtual class to construct an MDD representing an arbitrary Boolean function.
 * The represented function depends on the real class.
 */
class MDDBuilder {

protected:

	//Root node of the built MDD
	MDD * root;

	//Depth of the MDD
	int depth;

	//Number of nodes of the build MDD
	int nodeCount;


	bool longedges;


	virtual MDD * buildMDD() = 0;

public:

   //Constructor
	MDDBuilder();
	~MDDBuilder();

	MDD * getMDD();
	virtual MDD * addRoot(int k);

	int getSize() const;

	virtual void createGraphviz(ostream & os, vector<vector<int> > * labels = NULL) const = 0 ;

	vector<literal> asserted;
	vector<literal> elses;

};


#endif
