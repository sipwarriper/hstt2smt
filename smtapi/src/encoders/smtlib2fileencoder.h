#ifndef SMTLIB2FILEENCODER_DEFINITION
#define SMTLIB2FILEENCODER_DEFINITION

#include "fileencoder.h"

using namespace std;
using namespace smtapi;

/*
 * This class codifies an SMT formula into the SMT-LIB2 format.
 * If it cannot be written into the specified destination file,
 * the application, a panic exit will occur with error code BADFILE_ERROR.
 */
class SMTLIB2FileEncoder : public FileEncoder {

private:

	string solver;

	//Configuration parameters
	string logic;

	//Functions to avoid conflicts between Int and Boolean
	//unnamed variables:
		//Concatenates "i_" at the begining of a variable name
		inline string ivn(const string &s) const;
		//Concatenates "i_" at the begining of a variable name
		inline string bvn(const string &s) const;


	//Writes the codification of 'c' into 'os'
	void pclause(SMTFormula * f, const clause & c,ostream & os) const;
	//Writes the codification of 'l' into 'os'
	void pliteral(SMTFormula * f, const literal & l,ostream & os) const;
	//Writes the codification of 's' into 'os'
	void psum(SMTFormula * f, const intsum & s,ostream & os) const;
	//Writes the codification of 'p' into 'os'
	void pprod(SMTFormula * f, const intprod & p,ostream & os) const;


public:

	//Constructor by filename
	SMTLIB2FileEncoder(Encoding * enc, const string & solver);

	//Destructor
	~SMTLIB2FileEncoder();

	void createFile(ostream & os, SMTFormula * f) const;

	bool checkSAT(int lb, int ub);

	bool optimize(int lb, int ub);

};

#endif
