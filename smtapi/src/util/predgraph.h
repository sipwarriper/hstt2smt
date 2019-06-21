#ifndef PREDGRAPH_H
#define PREDGRAPH_H

#include <set>
#include <map>
#include <list>
#include <vector>
#include <cstdlib>
#include <limits.h>

using namespace std;

class PredGraph
{

private:
  
    // number of nodes
    int n;
    
    //Indirection to join the nodes
    vector<int> ind;
 
    // Set where each element belongs
    vector<set<int> > adj_list;
    
    
    //Coefficients involved in each set
    vector<vector<int> *> coefficients;

public:
    PredGraph(int n); // Constructor
    ~PredGraph(); // Destructor
    
    void addEdge(int a, int b);
    
    //Joins the set containing 'a' and the set containting 'b'.
    //Pre: 'a' and 'b' are in two different sets
    void join(int a, int b);
    
    int getCoincidences(int a, int b);
    
    void addCoefficient(int i, int coef);
        
    void greedyCoincidencesCover();
    
    void givenEdgesCover(list<pair<int,int> >edges);
    
    void getSets(vector<set<int> > & sets,const vector<int> & mapping);
    
};
 
#endif