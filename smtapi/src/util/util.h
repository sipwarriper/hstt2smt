#ifndef UTIL_DEFINITION
#define UTIL_DEFINITION

#include <vector>
#include <set>
#include <string>
#include <map>
#include <sstream>
#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include "smtapi.h"

using namespace std;
using namespace smtapi;

struct treenode{
	treenode * lchild;
	treenode * rchild;
	set<int> coefs;
	int idx;
	treenode(treenode *l, treenode *r, const set<int> & s){
		this->lchild = l;
		this->rchild = r;
		this->coefs = s;
		this->idx=-1;
	}
	treenode(int idx, const set<int> & s){
		this->idx = idx;
		this->lchild = NULL;
		this->rchild = NULL;
		this->coefs = s;
	}
	~treenode(){
		if(lchild!=NULL) delete lchild;
		if(rchild!=NULL) delete rchild;
	}
};

namespace util
{

void sortCoefsDecreasing(vector<int> & Q, vector<literal> & X);
void sortCoefsDecreasing(vector<vector<int> > & Q, vector<vector<literal> > & X);
void sortCoefsIncreasing(vector<int> & Q, vector<literal> & X);
void sortCoefsIncreasing(vector<vector<int> > & Q, vector<vector<literal> > & X);
void sortBySimilarityIncreasing(vector<vector<int> > & Q, vector<vector<literal> > & X);
void sortCoefsTotalizer(vector<vector<int> > & Q, vector<vector<literal> > & X, int K);
void sortCoefsTotalizerDifs(vector<vector<int> > & Q, vector<vector<literal> > & X);
void addElementsToOrder(treenode * node,
		const vector<vector<int> > & Q, const vector<vector<literal> > & X,
		vector<vector<int> > & Q2, vector<vector<literal> > & X2);
bool isPowerOf2(int x);
bool nthBit(int x, int n);
string getFileName(const string& s);

template<typename T>
void setOrder(vector<T> & data, int *order);

bool existsSorted(const vector<int> & v, int x);
void insertSorted(vector<int> & v, int x);

void floydWarshall(int ** adjMatrix, int size);

void reduceByEO(vector<vector<int> > & Q, vector<vector<literal> >& X, int & K);

void printAMOPB(const vector<vector<int> > & Q, const vector<vector<literal> > & X, int K);

void PBtoAMOPB(const vector<int> & Q, const vector<literal> & X, vector<vector<int> > & QQ, vector<vector<literal> > &XX);

void AMOPBtoPB(const vector<vector<int> > & Q, const vector<vector<literal> > &X, vector<int> & QQ, vector<literal> & XX);

int sum(const vector<int> & v);

template<typename TK, typename TV>
vector<TK> extract_keys(map<TK, TV> const& input_map) {
  vector<TK> retval;
  for (auto const& element : input_map)
    retval.push_back(element.first);
  return retval;
}

template<typename TK, typename TV>
vector<TV> extract_values(map<TK, TV> const& input_map) {
  vector<TV> retval;
  for (auto const& element : input_map)
    retval.push_back(element.second);
  return retval;
}

//Return true if 'vals' contains 'val'
inline bool defined(const set<string> & vals, const string & val){
	return vals.find(val)!=vals.end();
}

inline bool boolstring(const string & s){
	return s=="0" || s == "1";
}

inline bool isInteger(const string & s)
{
	if(s.empty() || ((!isdigit(s[0])) && (s[0] != '-') && (s[0] != '+')))
		return false ;

	char * p ;
	strtol(s.c_str(), &p, 10) ;

	return (*p == 0) ;
}

inline void show_list(const set<string> & vs){
	bool comma=false;
	for(const string & s : vs)
	{
		if(comma) cerr << ",";
		else comma=true;
		cerr << " " << s;
	}
	cerr << endl;
}

inline void show_list(const vector<string> & vs){
	bool comma=false;
	for(const string & s : vs)
	{
		if(comma) cerr << ",";
		else comma=true;
		cerr << " " << s;
	}
	cerr << endl;
}

}

#endif

