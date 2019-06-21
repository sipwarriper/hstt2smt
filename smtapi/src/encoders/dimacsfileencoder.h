#ifndef DIMACSFILEENCODER_DEFINITION
#define DIMACSFILEENCODER_DEFINITION

#include "fileencoder.h"
#include "smtformula.h"
#include <iostream>

using namespace std;
using namespace smtapi;



/*
 * This class codifies a SAT formula into the DIMACS format.
 * If the formula contains theory literals, a panic exit will
 * occur with error code BADCODIFICATION_ERROR. If it cannot be
 * written into the specified destination file, the application,
 * a panic exit will occur with error code BADFILE_ERROR.
 */
class DimacsFileEncoder : public FileEncoder {
private:
	
	string solver;
	
	void createSATFile(ostream & os, SMTFormula * f) const;

	void createMaxSATFile(ostream & os, SMTFormula * f) const;
	
	string getCall() const;
	
public:	
  
	//Constructor
	DimacsFileEncoder(Encoding * enc, const string &solver); 
	
	//Destructor
	~DimacsFileEncoder();

	virtual void createFile(ostream & os, SMTFormula * f) const;

	bool checkSAT(int lb, int ub);
	
	bool optimize(int lb, int ub);
	
};

#endif

