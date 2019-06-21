#include "dimacsfileencoder.h"
#include <iostream>
#include <fstream>
#include <cstdio>
#include <memory>
#include <stdexcept>
#include <array>
#include "errors.h"
#include <iterator>


using namespace std;
using namespace smtapi;

DimacsFileEncoder::DimacsFileEncoder(Encoding * enc, const string & solver) : FileEncoder(enc){
	if(solver=="yices")
		this->solver="yices-sat";
	else if(solver=="glucose")
		this->solver="glucose_release";
	else if(solver=="openwbo")
		this->solver="open-wbo_release";
	else{
		cerr << "Unsupported solver " << solver << endl;
		exit(BADARGUMENTS_ERROR);
	}
}

DimacsFileEncoder::~DimacsFileEncoder(){

}

string DimacsFileEncoder::getCall() const{
	if(produceModels()){
		if(solver == "glucose_release")
			return solver + " -model " + getTMPFileName() + " | grep -E '(^s )|(^v )'";
		else if(solver == "yices-sat")
			return solver + " -m " + getTMPFileName() + " | tail -n 2";
	}
	else if(solver == "glucose_release") //Apanyu momentani
		return solver + " " + getTMPFileName() + " | grep -E '(^s )|(^c CPU time)' | cut -d ':' -f 2 | sed -e 's/s//g'";
	else return solver + " " + getTMPFileName() + " | grep -E '(^s )|(^v )'";
}

bool DimacsFileEncoder::checkSAT(int lb, int ub){

	string filename = getTMPFileName();
	if(workingFormula.f==NULL){
		workingFormula.f = enc->encode(lb, ub);
		workingFormula.LB = lb;
		workingFormula.UB = ub;
	}
	else{
		bool narrowed = enc->narrowBounds(workingFormula,lastLB, lastUB, lb, ub);
		if(!narrowed){
			delete workingFormula.f;
			workingFormula.f = enc->encode(lb, ub);
			workingFormula.LB = lb;
			workingFormula.UB = ub;
		}
	}

	lastLB = lb;
	lastUB = ub;

	ofstream os(filename.c_str());
	if(!os.is_open()){
		cerr << "Error: could not open temporary working file: " << filename << endl;
		exit(BADFILE_ERROR);
	}

	createFile(os,workingFormula.f);

	os.close();

	std::array<char, 512> buffer;
	std::string result;
	clock_t begin_time = clock();

   std::shared_ptr<FILE> pipe(popen(getCall().c_str(),"r"), pclose);
   if (!pipe) throw std::runtime_error("popen() failed!");
   while (!feof(pipe.get())) {
        if (fgets(buffer.data(), 512, pipe.get()) != nullptr)
            result += buffer.data();
    }

	lastchecktime = ((float)( clock() - begin_time )) /  CLOCKS_PER_SEC;
	istringstream iss(result);
	std::vector<std::string> results((istream_iterator<std::string>(iss)),
                                 istream_iterator<std::string>());

	int i = 1;
	int j = 2;
	bool sat;

	if(!produceModels() && solver=="glucose_release"){
		solverchecktime = stof(results[0]);
		sat = results[1]=="SATISFIABLE";
	}
	else{
		if(results[1]=="SATISFIABLE"){
			sat = true;
		}
		else if(results[1]=="OPTIMUM"){
			sat = true;
			j++;
		}
		else
			sat=false;
	}


	if(sat && produceModels()){
		//Retrieve model
		vector<bool> model(workingFormula.f->getNBoolVars() + 1);
		while(j < results.size()){
			if(results[j]!="v"){
				model[i] = stoi(results[j]) > 0;
				i++;
			}
			j++;
		}
		enc->setModel(workingFormula,lb,ub,model,vector<int>());
	}
	remove(filename.c_str());
	return sat;

}


bool DimacsFileEncoder::optimize(int lb, int ub){

	if(workingFormula.f!=NULL)
		delete workingFormula.f;
	workingFormula.f = enc->encode(lb, ub);
	workingFormula.LB = lb;
	workingFormula.UB = ub;

	string filename = getTMPFileName();

	ofstream os(filename.c_str());
	if(!os.is_open()){
		cerr << "Error: could not open temporary working file: " << filename << endl;
		exit(BADFILE_ERROR);
	}

	createFile(os,workingFormula.f);

	std::array<char, 512> buffer;
   std::string result;
	clock_t begin_time = clock();
    std::shared_ptr<FILE> pipe(popen((solver + " " + filename + " | grep -E '(^s )|(^v )'").c_str(),"r"), pclose);
    if (!pipe) throw std::runtime_error("popen() failed!");
    while (!feof(pipe.get())) {
        if (fgets(buffer.data(), 512, pipe.get()) != nullptr)
            result += buffer.data();
    }

	lastchecktime = float( clock() - begin_time ) /  CLOCKS_PER_SEC;


	istringstream iss(result);
	std::vector<std::string> results((istream_iterator<std::string>(iss)),
                                 istream_iterator<std::string>());

	bool sat = results[1]=="OPTIMUM";
	if(sat && produceModels()){
		//Retrieve model
		vector<bool> model(workingFormula.f->getNBoolVars() + 1);
		int i = 1;
		int j = 3;
		while(j < results.size()){
			if(results[j]!="v"){
				model[i] = stoi(results[j]) > 0;
				i++;
			}
			j++;
		}
		enc->setModel(workingFormula,lb,ub,model,vector<int>());
	}

	remove(filename.c_str());

	return sat;
}


void DimacsFileEncoder::createFile(ostream & os, SMTFormula * f) const{

	switch(f->getType()){
		case SATFORMULA:
			createSATFile(os,f);
			break;
		case MAXSATFORMULA:
			createMaxSATFile(os,f);
			break;
		default:
			cerr << "Expected SAT or MaxSAT formula" << endl;
			exit(BADCODIFICATION_ERROR);
			break;
	}
}


void DimacsFileEncoder::createSATFile(ostream & os, SMTFormula * f) const{

	os << "p cnf " << f->getNBoolVars() << " " << f->getNClauses() << endl;
	for(const clause & c : f->getClauses()){
		for(const literal & l : c.v){
			if(l.arith){
				cerr << "Error: attempted to add arithmetic literal to SAT encodign"<< endl;
				exit(BADCODIFICATION_ERROR);
			}

			if(l.v.id <= 0 || l.v.id>f->getNBoolVars()){
				cerr << "Error: asserted undefined Boolean variable"<< endl;
				exit(UNDEFINEDVARIABLE_ERROR);
			}

			os << (l.sign ? l.v.id : -l.v.id) << " ";
		}
		os << "0" << endl;
	}
}

void DimacsFileEncoder::createMaxSATFile(ostream & os, SMTFormula * f) const{

	if(f->hasSoftClausesWithVars()){
		cerr << "Error: DIMACS does not accept soft clauses with associated variables" << endl;
		exit(BADCODIFICATION_ERROR);
	}

	int whard = f->getHardWeight();
	os << "p wcnf " << f->getNBoolVars() << " " << f->getNClauses() + f->getNSoftClauses() << " " << whard << endl;
	for(const clause & c : f->getClauses()){
		os << whard << " ";
		for(const literal & l : c.v){
			if(l.arith){
				cerr << "Error: attempted to add arithmetic literal to SAT encodign"<< endl;
				exit(BADCODIFICATION_ERROR);
			}

			if(l.v.id <= 0 || l.v.id>f->getNBoolVars()){
				cerr << "Error: asserted undefined Boolean variable"<< endl;
				exit(UNDEFINEDVARIABLE_ERROR);
			}

			os << (l.sign ? l.v.id : -l.v.id) << " ";
		}
		os << "0" << endl;
	}

	for(int i = 0; i < f->getNSoftClauses(); i++){
		const clause & c = f->getSoftClauses()[i];
		os << f->getWeights()[i] << " ";
		for(const literal & l : c.v){
			if(l.arith){
				cerr << "Error: attempted to add arithmetic literal to SAT encodign"<< endl;
				exit(BADCODIFICATION_ERROR);
			}

			if(l.v.id <= 0 || l.v.id>f->getNBoolVars()){
				cerr << "Error: asserted undefined Boolean variable"<< endl;
				exit(UNDEFINEDVARIABLE_ERROR);
			}

			os << (l.sign ? l.v.id : -l.v.id) << " ";
		}
		os << "0" << endl;
	}
}
