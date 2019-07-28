#ifndef SATBASICMODEL_H
#define SATBASICMODEL_H


#include "Model.h"
#include "encoding.h"

struct AssignmentEntry{
    std::string event_id;
    int duration;
    std::string time_id;
    AssignmentEntry(std::string e_id, int d, std::string t_id){
        event_id = e_id;
        duration = d;
        time_id = t_id;
    }
};

class SATBasicModel : public Model, public Encoding
{
public:
    SATBasicModel(std::string filename);
    ~SATBasicModel();

    SMTFormula * encode(int LB = 0, int UB = INT_MAX);

    void setModel(const EncodedFormula &ef, int lb, int ub, const vector<bool> &bmodel, const vector<int> &imodel);
    int getObjective() const;
    bool printSolution(ostream &os) const;

    std::vector<AssignmentEntry> get_time_assignments() const;
    void set_time_for_event(int event_num, int duration, Time* start_t);

    void prefer_resources_constraint(const std::set<std::string> &events_ids, const std::set<std::string> &resources_ids, const std::string &role);
    void prefer_times_constraint(const int &cost, const std::set<std::string> &events_ids, const std::set<std::string> &times_ids, int duration);
    void avoid_clashes_constraint(const int &cost, const std::set<std::string> &resources_ids);
    void split_events_constraint(const int &cost, const std::set<std::string> &events, const int &min, const int &max, const int &min_amount, const int &max_amount);
    void spread_events_constraint(const int &cost, const std::set<std::string> &event_groups, std::unordered_map<std::string, std::pair<int, int>> time_groups);
    //time_groups is a dictionary of tuples. where keys are time_group references and values are tuples of the kind(minimum, maximum)

    void avoid_unavailable_times_constraint(const int &cost, const std::set<std::string> &resources_ids, const std::set<std::string> &times_ids);
    void distribute_split_events_constraints(const int &cost, const std::set<std::string> &event_ids, const int &duration, const int &min, const int &max);
    void limit_idle_times_constraint(const int &cost, const std::set<std::string> &resources_ids, const std::set<std::string> &time_groups_ids, const int &min, const int &max);
    void cluster_busy_times_constraint(const int &cost, const std::set<std::string> &resources_ids, const std::set<std::string> &time_groups_ids, const int &min, const int &max);


    void on_parsed_events(bool spread_constraint=true); //declare all related variables, xhstt constrains are still to be known.


private:

    SMTFormula *formula_; //current formula
    std::vector<clause> clauses_;//current clauses list?

    std::unordered_map<int, std::vector<boolvar>> xt_;//xt --> key: event number, content: boolvar for time (active time)
    std::unordered_map<int, std::vector<boolvar>> xs_;//xs --> key: event number, content: boolvar for time (starting time)
    std::unordered_map<int, std::unordered_map<int, std::vector<boolvar>>> xd_;//xd --> key: event number, content: dictionary --> key: 1..event.duration+1, content: variable for starting time with duration key
    std::vector<std::pair<boolvar, int>> pseudoVars_; //pseudoVars --> vector with pairs of variables and their weights.

    std::unordered_map<int, std::set<int>> xr_; //xr --> key: resource num, content: set with all the events that demand such resource.


    std::unordered_map<int, std::vector<bool>> xt_Res_;
    std::unordered_map<int, std::vector<bool>> xs_Res_;
    std::unordered_map<int, std::unordered_map<int, std::vector<bool>>> xd_Res_;
    std::vector<bool> pseudoVars_Res_;

    int pseudoValue;





    //xs --> event starting times, dictionary.
    //xd --> event durations, dictionary.
    //xr --> dictionary containing, for each resource, all the events that demand such resource.


};

#endif // SATBASICMODEL_H
