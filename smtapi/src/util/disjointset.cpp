#include "disjointset.h"
#include <iostream>    

DisjointSet::DisjointSet(int n)
{
  sets.resize(n);
  for(int i = 0; i < n; i++){
    sets[i] = new set<int>();
    sets[i]->insert(i);
  }
}

DisjointSet::~DisjointSet()
{
    for(int i = 0; i < sets.size(); i++){
    set<int> * group = sets[i]; 
    if(group != NULL){
      for(int j : (*group))
	sets[j]=NULL;       
      delete group;
    }
  }
}
 
void DisjointSet::join(int a, int b)
{
    set<int> * first = sets[a];
    set<int> * second = sets[b];
        
    if(first->size() < second->size()){
      set<int> *aux = first;
      first = second;
      second = aux;
    }
    
    first->insert(second->begin(),second->end());
    
    for(int i : (*second))
      sets[i] = first;
    
    delete second;
}


void DisjointSet::getSets(vector<set<int> > & groups,const vector<int> & mapping)
{
  map<set<int> *,bool> inserted;
  for(set<int> * group : sets){
    if(!inserted[group]){
      set<int> aux;
      for(int i : *group)
	aux.insert(mapping[i]);
      groups.push_back(aux);       
      inserted[group]=true;
    }
  }
}
  
 
