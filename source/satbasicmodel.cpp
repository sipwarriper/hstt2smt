#include "satbasicmodel.h"
#include "util.h"
#include "errors.h"
#include <limits.h>
#include "XMLParser.h"
#include <unordered_map>
#include <algorithm>

SATBasicModel::SATBasicModel(std::string filename) : Model(){
    formula_ = new SMTFormula();


    auto parser = XMLParser(this, filename);
    parser.parse_model();


    //we need to remove duplicated clauses
    //TODO: dis

}

SATBasicModel::~SATBasicModel(){

}


void SATBasicModel::on_parsed_events(bool spread_constraint){
    //initialize xr_
    for(auto &r : resources_)
        xr_.insert({r.second->get_num(), std::set<int>()});

    int time_count = times_.size();

    for(auto & event_it : events_){
        Event *event = event_it.second;
        int e_num = event_it.second->get_num();
        std::vector<boolvar> xt_e(time_count);
        std::vector<boolvar> xs_e(time_count);

        //time variables;
        for(int i = 0; i<time_count; i++){
            xt_e[i] = formula_->newBoolVar("xt",e_num,i);
            xs_e[i] = formula_->newBoolVar("xs",e_num,i);
        }
        xt_.insert({e_num, xt_e});
        xs_.insert({e_num, xs_e});

        //preassigned times
        if (event->has_preassigned_time()){
            if (event->get_duration() != 1)
                throw ModelException("Encountered an Event with preassigned time with a duration great than 1");

            int preassigned = times_[event->get_time_ref()]->get_num();
            for (int t=0;t<time_count;t++){
                if(t != preassigned){
                    clause _c = !xt_[e_num][t];
                    clauses_.push_back(_c);
                }
            }
            clauses_.push_back(xt_[e_num][preassigned]);

        }
        else{
            std::vector<literal> _literals(time_count);
            for(int i = 0; i<time_count; i++) _literals[i] = xt_[e_num][i];
            formula_->addEK(_literals,event->get_duration());

            //Xs contained in Xt
            for(int i = 0; i<time_count; i++) clauses_.push_back(!xs_[e_num][i] | xt_[e_num][i]); //xs implies x
            //if event is taking place in xt and not in xt-1 then it must be a starting time
            for(int i = 1; i<time_count; i++) clauses_.push_back(!xt_[e_num][i] | xt_[e_num][i-1] | xs_[e_num][i]); //TODO: es podria afegir un exactly one per apurar aquesta clausula (potser ja hi es)

            //if an envent occurs at the first time slot, then it must be a starting time.
            clauses_.push_back(!xt_[e_num][0] | xs_[e_num][0]);

        }
        //xd constraints:
        //      i -> stands for the variables representin the starting times with duration i
        xd_[e_num] = std::unordered_map<int, std::vector<boolvar>>();
        for(int i = 1; i<event->get_duration()+1; i++){
            std::vector<boolvar> xd_e_i(time_count);
            for (int j=0;j<time_count;j++) xd_e_i[j] = formula_->newBoolVar("xd",e_num,i,j);
            xd_[e_num][i] = xd_e_i;

            for(int j=0;j<time_count;j++) clauses_.push_back(!xd_[e_num][i][j] | xs_[e_num][j]);

            if(!spread_constraint){
                //time immediately after an event of duration d, must be set to false
                for (int j=0; j<time_count-i; j++) clauses_.push_back(!xd_[e_num][i][j] | !xt_[e_num][j+i]);

                //also negate all unfeasible times (the laste ones)
                for(int j=time_count-i+1;j<time_count;j++) clauses_.push_back(!xd_[e_num][i][j]);
            }
            // when a bit of xd[e][i] make sure it takes palce in i consecutive slots
            for(int t = 0; t<time_count-i+1; t++){
                for(int ti = t; ti<t+i; ti++)
                    clauses_.push_back(!xd_[e_num][i][t] | xt_[e_num][ti]);
            }

            //if an event wit duration d i taking place, make sure no other events start while the event is taking place.
            if(i>1){ //TODO: revisar
                for(int t = 0; t<time_count-i+1; t++){
                    for(int ti = t+1; ti<t+i; ti++)
                        clauses_.push_back(!xd_[e_num][i][t] | !xs_[e_num][ti]);
                }
                for(int t = 0; t<time_count-i+1; t++){
                    for(int di = 1; di<i; di++){
                        for(int ti = t; ti<t+i; ti++)
                            clauses_.push_back(!xd_[e_num][i][t] | !xd_[e_num][di][ti]);
                    }
                }
            }
        }
        //If an event starts at time t, then exactly one Xd must be set
        if(event->get_duration() > 1){
            for(int t = 0; t<time_count; t++){
                std::vector<literal> _lts;
                for(int i=1;i<event->get_duration()+1; i++) _lts.push_back(xd_[e_num][i][t]);
                _lts.push_back(!xs_[e_num][t]);
                formula_->addEO(_lts);
            }
        }
        else{
            std::vector<clause> _cls;
            for(int i=1;i<time_count; i++)
                clauses_.push_back(!xs_[e_num][i] | xd_[e_num][1][i]);
        }

        //resource variables (this model requires all resources to be preassigned)
        //check that the event has all resources preassigned
        auto preassigned_resources  = event->get_preassigned_resources();
        if (preassigned_resources.size() > event->get_resources().size()) throw ModelException("At least one event with one resource left to assign");

        //add event ref to all preassigned resources entries in Xr
        for (auto & res : preassigned_resources)
            xr_[res->get_num()].insert(e_num);

    }

}



void SATBasicModel::set_time_for_event(int event_num, int duration, Time* start_t){
    //TODO mirarse si cal, sembla q no
}

void SATBasicModel::prefer_resources_constraint(const std::set<std::string> &events_ids, const std::set<std::string> &resources_ids, const std::string &role){
    //no nhi ha en el brazil instances, aixi q de moment no cal
}
void SATBasicModel::prefer_times_constraint(const int &cost, const std::set<std::string> &events_ids, const std::set<std::string> &times_ids, int duration){
    int time_count = times_.size();

    //create a set with all forbidden time slots
    std::set<int> forbidden;
    for (int i=0; i<time_count; i++){
        if(times_ids.find(num2time_[i]->get_identifier()) == times_ids.end())
            forbidden.insert(i);
    }

    float weight = cost/events_ids.size();

    //for each event, forbid all forbidden times for the given duration (if any)
    for (auto &event_id: events_ids){
        Event *event = events_[event_id];
        int e = event->get_num();
        if (duration<0){
            for (int d=1; d<event->get_duration()+1; d++){
                for(auto &t : forbidden){
                    boolvar x = formula_->newBoolVar("prefer_times", e,d,t);
                    clauses_.push_back(!xd_[e][d][t] | x);
                    pseudoVars_.insert({x, weight});
                }
            }
            for(auto &t : forbidden){
                boolvar x1 = formula_->newBoolVar("prefer_times_1", e, t);
                boolvar x2 = formula_->newBoolVar("prefer_times_2", e, t);
                clauses_.push_back(!xt_[e][t] | x1);
                clauses_.push_back(!xs_[e][t] | x2);
                pseudoVars_.insert({x1, weight});
                pseudoVars_.insert({x2, weight});
            }
        }
        else if (xd_[e].find(duration)!= xd_[e].end()){
            for(auto &t : forbidden){
                boolvar x = formula_->newBoolVar("prefer_times", e, t);
                clauses_.push_back(!xd_[e][duration][t] | x);
                pseudoVars_.insert({x,weight});
            }
        }
    }

}
void SATBasicModel::avoid_clashes_constraint(const int &cost, const std::set<std::string> &resources_ids){
    int time_count = times_.size();
    float weight = cost/resources_ids.size();
    for (auto &r_id: resources_ids){
        Resource *resource = resources_[r_id];
        std::set<int> requiring_events = xr_[resource->get_num()];
        for(int t = 0; t<time_count; t++)
            for(auto &e: requiring_events){
                boolvar i_var = formula_->newBoolVar("avoid_clashes", t, e);
                pseudoVars_.insert({i_var, weight});
                clauses_.push_back(xt_[e][t] | i_var);
            }
    }
}
void SATBasicModel::split_events_constraint(const int &cost, const std::set<std::string> &events, const int &min, const int &max, const int &min_amount, const int &max_amount){
    int time_count = times_.size();
    float _w = cost/events.size();
    float weight = std::max(static_cast<float>(1), _w);

    for (auto & event_id : events){
        Event * event = events_[event_id];
        int e = event->get_num();

        //nullify all durations below minimum
        for (int d = 1; d<min; d++){
            for(int t = 0; t<time_count; t++){
                boolvar x = formula_->newBoolVar("split_events", e, d, t);
                clauses_.push_back(!xd_[e][d][t] | x);
                pseudoVars_.insert({x, weight});
            }
        }
        //nullify all durations above maximum
        for(int d = max+1; d<event->get_duration()+1; d++){
            for(int t = 0; t<time_count; t++){
                boolvar x = formula_->newBoolVar("split_events", e, d, t);
                clauses_.push_back(!xd_[e][d][t] | x);
                pseudoVars_.insert({x, weight});
            }
        }

        if(min_amount == max_amount && min_amount>0){
            //exactly_k de xs_[e]
        }
        else{
            if(min_amount>0){
                //atleast k de xs_[e]
            }
            if(max_amount < time_count){
                //atmost k de xs[e]
            }
        }
    }


}
void SATBasicModel::spread_events_constraint(const int &cost, const std::set<std::string> &event_groups, std::unordered_map<std::string, std::pair<int, int>> time_groups){

}
void SATBasicModel::avoid_unavailable_times_constraint(const int &cost, const std::set<std::string> &resources_ids, const std::set<std::string> &times_ids){

}
void SATBasicModel::distribute_split_events_constraints(const int &cost, const std::set<std::string> &event_ids, const int &duration, const int &min, const int &max){

}
void SATBasicModel::limit_idle_times_constraint(const int &cost, const std::set<std::string> &resources_ids, const std::set<std::string> &time_groups_ids, const int &min, const int &max){

}
void SATBasicModel::cluster_busy_times_constraint(const int &cost, const std::set<std::string> &resources_ids, const std::set<std::string> &time_groups_ids, const int &min, const int &max){

}



SMTFormula * SATBasicModel::encode(int LB, int UB){
    return formula_;
}

void SATBasicModel::setModel(const EncodedFormula &ef, int lb, int ub, const vector<bool> &bmodel, const vector<int> &imodel){

}
int SATBasicModel::getObjective() const{
    return 0;
}
bool SATBasicModel::printSolution(ostream &os) const{

}


