#ifndef DISJOINTSET_H
#define DISJOINTSET_H

#include <set>
#include <map>
#include <list>
#include <vector>
#include <cstdlib>
#include <limits.h>

using namespace std;

class DisjointSet
{

private:
  
    // number of elements of all sets
    int n;
 
    // Set where each element belongs
    vector<set<int> *> sets;


public:
    DisjointSet(int n); // Constructor
    ~DisjointSet(); // Destructor
    
    //Joins the set containing 'a' and the set containting 'b'.
    //Pre: 'a' and 'b' are in two different sets
    void join(int a, int b);
            
    void getSets(vector<set<int> > & sets,const vector<int> & mapping);


    
};
#endif