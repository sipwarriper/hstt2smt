#ifndef SOLVINGARGUMENTS_DEFINITION
#define SOLVINGARGUMENTS_DEFINITION


#include <vector>
#include <set>
#include <string>
#include <map>
#include <sstream>
#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include "encoder.h"
#include "fileencoder.h"
#include "encoding.h"
#include "optimizer.h"
#include "smtformula.h"
#include "arguments.h"
#include "util.h"

#define programversion "1.0"

using namespace std;
using namespace smtapi;
using namespace util;
using namespace arguments;


enum SolvingArg {
	OUTPUT_ENCODING,
	FILE_FORMAT,
	PRODUCE_MODELS,
	SOLVER,
	OPTIMIZER,
	RANDOM_SEED,
	FILE_PREFIX,
	USE_API,
	USE_ASSUMPTIONS,
	NARROW_BOUNDS,
	USE_IDL_SOVER,
	PRINT_NOOPTIMAL_SOLUTIONS,
	PRINT_OPTIMAL_SOLUTION,
	PRINT_CHECKS,
	PRINT_CHECKS_STATISTICS,
	LOWER_BOUND,
	UPPER_BOUND,

	AMO_ENCODING,
	CARDINALITY_ENCODING,
	PB_ENCODING,
	AMOPB_ENCODING
};


class SolvingArguments : public Arguments<SolvingArg>{


private:

	SolvingArguments();

	static map<string,AMOEncoding> amoencodings;
	static map<string,CardinalityEncoding> cardinalityencodings;
	static map<string,PBEncoding> pbencodings;
	static map<string,AMOPBEncoding> amopbencodings;


public:

	virtual ~SolvingArguments();

	template<class OptionT>
	static SolvingArguments * readArguments(int argc, char ** argv, Arguments<OptionT> * pargs);

	void checkSolvingArguments();

	void printVersion() const;
	void printHelp() const;
	void printSolvingOptions() const;
	template<class OptionT>
	void printHelp(Arguments<OptionT> * pargs) const;


	Encoder * getEncoder(Encoding * enc);
	FileEncoder * getFileEncoder(Encoding * enc);
	Optimizer * getOptimizer();


	AMOEncoding getAMOEncoding();
	CardinalityEncoding getCardinalityEncoding();
	PBEncoding getPBEncoding();
	AMOPBEncoding getAMOPBEncoding();


};

template<class OptionT>
SolvingArguments * SolvingArguments::readArguments(int argc, char ** argv, Arguments<OptionT> * pargs){

	SolvingArguments * sargs = new SolvingArguments();
	sargs->addArgument(argv[0]);

	for(int i = 1; i < argc; i++){

		//If it is an argument
		if(strncmp(argv[i], "-", 1)!=0)
			pargs->addArgument(argv[i]);

		//If it is an OPTION
		else{
			string argval = argv[i];

			if(argval=="-h" || argval=="--help"){
				sargs->printHelp(pargs);
				exit(0);
			}

			if(argval=="-v" || argval=="--version"){
				sargs->printVersion();
				exit(0);
			}

			string arg = argval;
			int delpos = argval.find("=");
			if(delpos==argval.size()-1)
				arg = argval.substr(0,delpos);
			if(delpos==string::npos || delpos==argval.size()-1){
				cerr << "Missing value for argument " << arg << endl;
				exit(BADARGUMENTS_ERROR);
			}

			arg = argval.substr(0,delpos);
			string val = argval.substr(delpos+1,argval.size());

			bool knownarg=false;

			if(sargs->hasOption(arg)){
				knownarg = true;
				SolvingArg sarg = sargs->getOptionRef(arg);
				switch(sargs->getOptionType(sarg)){

					case INT_TYPE:
						if(!isInteger(val)){
							cerr << arg << " must be an integer value, received: " << val << endl;
							exit(BADARGUMENTS_ERROR);
						}
						sargs->setOption(sarg,stoi(val));
						break;

					case BOOL_TYPE:
						if(!boolstring(val)){
							cerr << arg << " must be either 0 or 1, received: " << val << endl;
							exit(BADARGUMENTS_ERROR);
						}
						sargs->setOption(sarg,val == "1");
						break;

					case STRING_TYPE:
						sargs->setOption(sarg,val);
						break;

					default:
						cerr << "Undefined type for option " << arg << endl;
						exit(BADARGUMENTS_ERROR);
						break;
				}
			}
			if(pargs->hasOption(arg)){
				knownarg = true;
				OptionT parg = pargs->getOptionRef(arg);
				switch(pargs->getOptionType(parg)){
					case INT_TYPE:
						if(!isInteger(val)){
							cerr << arg << " must be an integer value, received: " << val << endl;
							exit(BADARGUMENTS_ERROR);
						}
						pargs->setOption(parg,stoi(val));
						break;

					case BOOL_TYPE:
						if(!boolstring(val)){
							cerr << arg << " must be either 0 or 1, received: " << val << endl;
							exit(BADARGUMENTS_ERROR);
						}
						pargs->setOption(parg,val == "1");
						break;

					case STRING_TYPE:
						pargs->setOption(parg,val);
						break;

					default:
						cerr << "Undefined type for option " << arg << endl;
						exit(BADARGUMENTS_ERROR);
						break;
				}
			}

			if(!knownarg){
				cerr << "Undefined option " << arg << endl;
				exit(BADARGUMENTS_ERROR);
			}
		}
	}

	sargs->checkSolvingArguments();

	if(pargs->getNArguments() > pargs->getAllowedArguments()){
		cerr << "Extra arguments:";
		for(int i = pargs->getAllowedArguments(); i < pargs->getNArguments(); i++)
			cerr << " " << pargs->getArgument(i);

		cerr << endl << endl;
		cerr << "Run \"" << argv[0] << " -h\" for help" << endl;
		exit(BADARGUMENTS_ERROR);
	}
	else if(pargs->getNArguments() < pargs->getMinimumArguments()){
		cerr << "Missing arguments" << endl << endl;
		cerr << "Run \"" << argv[0] << " -h\" for help" << endl;
		exit(BADARGUMENTS_ERROR);
	}

	return sargs;
}

template<class OptionT>
void SolvingArguments::printHelp(Arguments<OptionT> * pargs) const{
	printVersion();
	cout << endl;

	cout << "SYNOPSIS:" << endl << endl;

	cout << "    " << getArgument(0) << " [OPTIONS]";

	for(int i = 0; i < pargs->getMinimumArguments(); i++)
		cout << " " << pargs->getArgumentName(i);

	for(int i = pargs->getMinimumArguments()+1; i < pargs->getAllowedArguments(); i++){
		cout << " [" << pargs->getArgumentName(i) << "]";
	}

	cout << endl << endl;

	if(pargs->getDescription() != ""){
		cout << "DESCRIPTION:" << endl << endl;
		cout << "  " << pargs->getDescription();
		cout << endl << endl;
	}

	if(pargs->getAllowedArguments() > 0){
		cout << "ARGUMENTS:" << endl << endl;
		for(int i = 0; i < pargs->getAllowedArguments(); i++){
			printf("  %s: %s\n\n", pargs->getArgumentName(i).c_str(), pargs->getArgumentDesc(i).c_str());
		}

		cout << endl;
	}

	cout << "OPTIONS:" << endl << endl;
	cout << "Generic program information" << endl << endl;
	printf("  -%-6s --%s","h","help\n");
	cout << "            Print help page." << endl << endl;
	printf("  -%-6s --%s","v","version\n");
	cout << "            Print program version." << endl << endl;

	if(pargs->getNOptions() > 0){
		cout << "Program specific" << endl << endl;
		pargs->printOptions();
	}

	cout << "Solving" << endl << endl;
	printOptions();
}


#endif

