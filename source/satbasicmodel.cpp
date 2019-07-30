#include "satbasicmodel.h"
#include "util.h"
#include "errors.h"
#include "XMLParser.h"
#include "TextTable.h"
#include <limits.h>
#include <unordered_map>
#include <algorithm>
#include <unordered_set>

SATBasicModel::SATBasicModel(std::string filename) : Model(){
    formula_ = new SMTFormula();
    pseudoValue = INT_MAX;

    auto parser = XMLParser(this, filename);
    parser.parse_model();
    week_days_dict = {
        {'o',0},//mOnday
        {'u',1},//tUesday
        {'e',2},//wEdneseday
        {'h',3},//tHursday
        {'r',4}//fRiday
    };
}

SATBasicModel::~SATBasicModel(){

}


std::vector<AssignmentEntry> SATBasicModel::get_time_assignments() const{
    int time_count = times_.size();
    std::vector<AssignmentEntry> result;
    for (auto & event : events_){
        for (int d = 1; d<event.second->get_duration()+1; d++){
            for(int i=0; i<time_count; i++){
                int e_num = event.second->get_num();
                bool var = xd_Res_.find(e_num)->second.find(d)->second[i];
                if(var){
                    AssignmentEntry entry(event.first, d, num2time_.find(i)->second->get_identifier());
                    result.push_back(entry);
                }
            }
        }
    }
    return result;
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
                    clauses_.push_back(!xt_[e_num][t]);
                }
            }
            clauses_.push_back(xt_[e_num][preassigned]);

        }
        else{
            std::vector<literal> _literals(time_count);
            for(int i = 0; i<time_count; i++) _literals[i] = xt_[e_num][i];
            formula_->addEK(_literals,event->get_duration());

            //Xs contained in Xt
            for(int i = 0; i<time_count; i++)
                clauses_.push_back(!xs_[e_num][i] | xt_[e_num][i]); //xs implies x
            //if event is taking place in xt and not in xt-1 then it must be a starting time
            for(int i = 1; i<time_count; i++)
                clauses_.push_back(!xt_[e_num][i] | xt_[e_num][i-1] | xs_[e_num][i]); //TODO: es podria afegir un exactly one per apurar aquesta clausula (potser ja hi es)

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
            if(i>1){
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

    int weight = cost/events_ids.size();

    //for each event, forbid all forbidden times for the given duration (if any)
    for (auto &event_id: events_ids){
        Event *event = events_[event_id];
        int e = event->get_num();
        if (duration<0){
            for (int d=1; d<event->get_duration()+1; d++){
                for(auto &t : forbidden){
                    clause cl = !xd_[e][d][t];
                    if(cost>=0){
                        boolvar x = formula_->newBoolVar("prefer_times", e,d,t);
                        pseudoVars_.push_back({x, weight});
                        cl = cl | x;
                    }
                    clauses_.push_back(cl);
                }
            }
            for(auto &t : forbidden){
                clause cl1 = !xt_[e][t];
                clause cl2 = !xs_[e][t];
                if(cost>=0){
                    boolvar x1 = formula_->newBoolVar("prefer_times_1", e, t);
                    boolvar x2 = formula_->newBoolVar("prefer_times_2", e, t);
                    cl1 = cl1 | x1;
                    cl2 = cl2 | x2;
                    pseudoVars_.push_back({x1, weight});
                    pseudoVars_.push_back({x2, weight});
                }
                clauses_.push_back(cl1);
                clauses_.push_back(cl2);
            }
        }
        else if (xd_[e].find(duration)!= xd_[e].end()){
            for(auto &t : forbidden){
                clause cl = !xd_[e][duration][t];
                if(cost>=0){
                    boolvar x = formula_->newBoolVar("prefer_times", e, t);
                    cl = cl | x;
                    pseudoVars_.push_back({x,weight});
                }
                clauses_.push_back(cl);
            }
        }
    }

}
void SATBasicModel::avoid_clashes_constraint(const int &cost, const std::set<std::string> &resources_ids){
    //Problematic
    int time_count = times_.size();
    int weight = cost/resources_ids.size();
    for (auto &r_id: resources_ids){
        Resource *resource = resources_[r_id];
        std::set<int> requiring_events = xr_[resource->get_num()];
        for(int t = 0; t<time_count; t++){
            std::vector<literal> _lts;
            for(auto &e: requiring_events){
                _lts.push_back(xt_[e][t]);
            }
            if(cost<0){
                formula_->addAMO(_lts);
            }
            else{
                boolvar x = formula_->newBoolVar("avoid_clashes",t);
                formula_->addAMKWithCheckVar(_lts, 1, x);
                pseudoVars_.push_back({x,weight});
            }
        }
    }
}
void SATBasicModel::split_events_constraint(const int &cost, const std::set<std::string> &events, const int &min, const int &max, const int &min_amount, const int &max_amount){
        int time_count = times_.size();
        float _w = cost/events.size();
        int weight = std::max(static_cast<float>(1), _w);

        for (auto & event_id : events){
            Event * event = events_[event_id];
            int e = event->get_num();

            //nullify all durations below minimum
            for (int d = 1; d<min; d++){
                for(int t = 0; t<time_count; t++){
                    clause cl = !xd_[e][d][t];
                    if (cost >= 0){
                        boolvar x = formula_->newBoolVar("split_events", e, d, t);
                        cl = cl | x;
                        pseudoVars_.push_back({x, weight});
                    }
                    clauses_.push_back(cl);
                }
            }
            //nullify all durations above maximum
            for(int d = max+1; d<event->get_duration()+1; d++){
                for(int t = 0; t<time_count; t++){
                    clause cl = !xd_[e][d][t];
                    if(cost >= 0){
                        boolvar x = formula_->newBoolVar("split_events", e, d, t);
                        cl = cl | x;
                        pseudoVars_.push_back({x, weight});
                    }
                    clauses_.push_back(cl);
                }
            }
            std::vector<literal> vec_;
            vec_.insert(vec_.end(), xs_[e].begin(), xs_[e].end());

            if(min_amount == max_amount && min_amount>0){
                //exactly_k de xs_[e]
                if (cost>=0){
                    boolvar x = formula_->newBoolVar("split_events", e);
                    formula_->addEKWithCheckVar(vec_, min_amount,x);
                    pseudoVars_.push_back({x,weight});
                }
                else{
                     formula_->addEK(vec_, min_amount);
                }
            }
            else{
                if(min_amount>0){
                    //atleast k de xs_[e]
                    if(cost>=0){
                        boolvar x = formula_->newBoolVar("split_events_min", e);
                        formula_->addALKWithCheckVar(vec_, min_amount,x);
                        pseudoVars_.push_back({x,weight});
                    }
                    else{
                         formula_->addALK(vec_, min_amount);
                    }
                }
                if(max_amount < time_count){
                    //atmost k de xs[e]
                    if(cost>=0){
                        boolvar x = formula_->newBoolVar("split_events_max", e);
                        formula_->addAMKWithCheckVar(vec_, max_amount,x);
                        pseudoVars_.push_back({x,weight});
                    }
                    else {
                         formula_->addAMK(vec_, max_amount);
                    }
                }
            }
        }
}
void SATBasicModel::spread_events_constraint(const int &cost, const std::set<std::string> &event_groups, std::unordered_map<std::string, std::pair<int, int>> time_groups){
    //problematic
    int time_count = times_.size();

    float _w = cost/event_groups.size();
    int weight = std::max(static_cast<float>(1), _w);

    std::vector<std::vector<int>> time_ranges;

    for(auto & time_group_it : time_groups){
        Group *time_group = time_groups_[time_group_it.first];
        int min = time_group_it.second.first;
        int max = time_group_it.second.second;
        std::vector<int> time_nums;
        for (std::string time_id : time_group->get_elems())
            time_nums.push_back(times_[time_id]->get_num());
        std::sort(time_nums.begin(), time_nums.end());

        //generate a list with the time_nums that do not have an immediat predecessor within the group
        // and impose that for those times, if xt_ then xs_ (that is, those times will always be starting times)
        std::vector<int> heads;
        for(int i = 0; i<time_nums.size(); i++){
            int prev = i==0?time_nums.size()-1:i-1;
            if(time_nums[i] != time_nums[prev]+1) heads.push_back(time_nums[i]);
        }
        int i = 0;
        while(i < time_nums.size()-1){
            std::vector<int> r;
            while (i< time_nums.size()-1 && time_nums[i] == time_nums[i+1]-1){
                r.push_back(time_nums[i]);
                i++;
            }
            r.push_back(time_nums[i]);
            i++;
            time_ranges.push_back(r);
        }

        for(std::string event_group_id : event_groups){
            Group * event_group = event_groups_[event_group_id];
            std::vector<literal> z_args;
            std::set<std::string> elems = event_group->get_elems();
            for (std::string event_id : elems){
                int e = events_[event_id]->get_num();
                //impose that for headtimes, if xt then xs such that all starting times of the time group are marked as such in xs if they are set in xt.
                for(int t : heads)
                    clauses_.push_back(!xt_[e][t] | xs_[e][t]);
                for(int t : time_nums)
                    z_args.push_back(xs_[e][t]);
            }
            if(min == max & min>0){
                if(cost>=0){
                    boolvar x = formula_->newBoolVar("spread_events_"+event_group_id);
                    formula_->addEKWithCheckVar(z_args, min, x);
                    pseudoVars_.push_back({x,weight});
                }
                else{
                    formula_->addEK(z_args, min);
                }
            }
            else{
                if(min>0){
                    if(cost>=0){
                        boolvar x = formula_->newBoolVar("spread_events_min_"+event_group_id);
                        formula_->addALKWithCheckVar(z_args, min, x);
                        pseudoVars_.push_back({x,weight});
                    }
                    else{
                        formula_->addALK(z_args, min);
                    }
                }
                if(max < z_args.size()){
                    if(cost>=0){
                        boolvar x = formula_->newBoolVar("spread_events_max_"+event_group_id);
                        formula_->addAMKWithCheckVar(z_args, max, x);
                        pseudoVars_.push_back({x,weight});
                    }
                    else{
                        formula_->addAMK(z_args, max);
                    }
                }
            }
        }
    }

    //time_ranges will contain a list of all consecutive time ranges, so for each event
    for(auto & event_it : events_){
        Event* event = event_it.second;
        int e = event->get_num();
        for (std::vector<int> tr : time_ranges){
            for(int d = 1; d < event->get_duration()+1; d++){
                //initial, if the event is scheduled with duration d, then time t+d must be free
                for(int i = 0; i<tr.size()-d; i++){
                    //clauses_.push_back(!xd_[e][d][tr[i]] | !xt_[e][tr[i]+d]);
                    std::vector<literal> _v;
                    for (int t = tr[i]; t<tr[i]+d; t++)
                        _v.push_back(!xt_[e][t]);
                    _v.push_back(xt_[e][tr[i]+d]);
                    if (i>0)
                        _v.push_back(xt_[e][tr[i-1]]);
                    _v.push_back(xd_[e][d][tr[i]]);
                    clause cl(_v);
                    clauses_.push_back(cl);
                }
                //final if all the last d times of a time_range are set, then xd must be also set.
                if(tr.size()-d >=0){
                    std::vector<literal> _v;
                    for(int t = tr[tr.size()-d]; t<tr[tr.size()-1]+1; t++)
                        _v.push_back(!xt_[e][t]);
                    if (tr.size()-d > 0)
                        _v.push_back(xt_[e][tr[tr.size()-d-1]]);
                    _v.push_back(xd_[e][d][tr[tr.size()-d]]);
                    clause cl(_v);
                    clauses_.push_back(cl);
                }
                //also nullify all unfeasible times.
                int ini = std::max(static_cast<int>(tr.size()-d+1), 0);
                for(int i = ini; i< tr.size(); i++)
                    clauses_.push_back(!xd_[e][d][tr[i]]);
            }
        }
    }

}
void SATBasicModel::avoid_unavailable_times_constraint(const int &cost, const std::set<std::string> &resources_ids, const std::set<std::string> &times_ids){
    int weight = cost/resources_ids.size();

    std::set<int> unavaliable_times;
    for (auto & time_id : times_ids)
        unavaliable_times.insert(times_[time_id]->get_num());
    for (auto & resource_id : resources_ids){
        //get all events that the resource are to attend
        for (int e : xr_[resources_[resource_id]->get_num()]){
            for(int t: unavaliable_times){
                clause cl = !xt_[e][t];
                if(cost>=0){
                    boolvar x = formula_->newBoolVar("avoid_unavailable_times"+resource_id, e, t);
                    cl = cl | x;
                    pseudoVars_.push_back({x, weight});
                }
                clauses_.push_back(cl);
            }
        }

    }
}
void SATBasicModel::distribute_split_events_constraints(const int &cost, const std::set<std::string> &event_ids, const int &duration, const int &min, const int &max){
    //mirarsho amb en suy
}
void SATBasicModel::limit_idle_times_constraint(const int &cost, const std::set<std::string> &resources_ids, const std::set<std::string> &time_groups_ids, const int &min, const int &max){
    int w_ = cost/resources_ids.size();
    int weight = std::max(1, w_);
    for (std::string resource_id : resources_ids){
        int r = resources_[resource_id]->get_num();
        std::vector<literal> idle_vars;
        for (std::string time_group_id:time_groups_ids){
            std::set<std::string> time_ids = time_groups_[time_group_id]->get_elems();
            std::unordered_set<int> time_nums;
            int min_time = INT_MAX;
            int max_time = 0;
            for (std::string time_id : time_ids){
                int t_num = times_[time_id]->get_num();
                time_nums.insert(t_num);
                if (min_time>t_num) min_time = t_num;
                if (max_time<t_num) max_time = t_num;
            }
            //check compactness
            for (int t = min_time+1; t<max_time; t++)
                if (time_nums.find(t) == time_nums.end())
                    throw ModelException("Encountered a non compact TimeGroup in LimitIdleTimesConstraint");
            //get a set with all the eligible times (times eligible to be IDLE inside the time group)
            time_nums.extract(min_time);
            time_nums.extract(max_time);

            for (int t : time_nums){
                std::vector<boolvar> before; //get all vars that represent time_nums that go before this time
                std::vector<boolvar> after; //get all vars that represent time_nums that go after this time
                std::vector<boolvar> current; //get all vars that represent time_num 't'
                for  (int e : xr_[r]){
                    for (int tb = min_time; tb<t; tb++)
                        before.push_back(xt_[e][tb]);
                    for (int ta = t+1; ta< max_time; ta++)
                        after.push_back(xt_[e][ta]);
                    current.push_back(xt_[e][t]);
                }

                //for each elegible_time create an auxiliary variable that will remain true if the time is iddle.
                //Note: to loewer the amount of clauses the condition will relax if the minimum equals to 0, in such case, the auxiliary var will remain true if the time is idle, otherwise unknown.
                boolvar idle = formula_->newBoolVar("idle_"+time_group_id+"_"+resource_id, t);
                for(boolvar curr : current)
                    for (boolvar b : before)
                        for (boolvar a : after)
                            clauses_.push_back(curr | !a | !b | idle);
                if(min>0){
                    clause c1 = !idle, c2 = !idle;
                    for (boolvar a: after)
                        c1 = c1 | a;
                    for(boolvar b: before)
                        c2 = c2 | b;
                    clauses_.push_back(c1);
                    clauses_.push_back(c2);
                    for (boolvar curr: current)
                        clauses_.push_back(!idle | !curr);
                }
                idle_vars.push_back(idle);
            }
        }
        if(min == max & min>0){
            if(cost>=0){
                boolvar x = formula_->newBoolVar("limit_idle_"+resource_id);
                formula_->addEKWithCheckVar(idle_vars, min, x);
                pseudoVars_.push_back({x,weight});
            }
            else{
                formula_->addEK(idle_vars, min);
            }
        }
        else{
            if(min>0){
                if(cost>=0){
                    boolvar x = formula_->newBoolVar("limit_idle_min_"+resource_id);
                    formula_->addALKWithCheckVar(idle_vars, min, x);
                    pseudoVars_.push_back({x,weight});
                }
                else{
                    formula_->addALK(idle_vars, min);
                }

            }
            if(max < idle_vars.size()){
                if(cost>=0){
                    boolvar x = formula_->newBoolVar("limit_idle_max_"+resource_id);
                    formula_->addAMKWithCheckVar(idle_vars, max, x);
                    pseudoVars_.push_back({x,weight});
                }
                else{
                    formula_->addAMK(idle_vars, max);
                }

            }
        }
    }

}
void SATBasicModel::cluster_busy_times_constraint(const int &cost, const std::set<std::string> &resources_ids, const std::set<std::string> &time_groups_ids, const int &min, const int &max){
    int w_ = cost/resources_ids.size();
    int weight = std::max(1, w_);
    int time_count = times_.size();
    for (std::string r_id : resources_ids){
        int r = resources_[r_id]->get_num();

        //vector containing all auxiliary variables
        std::vector<literal> busies;
        for (std::string time_group_id : time_groups_ids){
            std::set<string> time_ids = time_groups_[time_group_id]->get_elems();
             //rt_args contains all xt_ variables bounded to r
            std::vector<boolvar> rt_args;
            for (int e : xr_[r])
                for(std::string t_id : time_ids)
                    rt_args.push_back(xt_[e][times_[t_id]->get_num()]);
            boolvar busy = formula_->newBoolVar("busy_"+time_group_id, r);
            clause cl = !busy;
            for (boolvar b : rt_args){
                cl = cl | b;
                clauses_.push_back(!b | busy);
            }
            clauses_.push_back(cl);
            busies  .push_back(busy);
        }
        if(min == max & min>0){
            if(cost>=0){
                boolvar x = formula_->newBoolVar("cluster_busy_times", r);
                formula_->addEKWithCheckVar(busies, min, x);
                pseudoVars_.push_back({x,weight});
            }
            else{
                formula_->addEK(busies, min);
            }
        }
        else{
            if(min>0){
                if(cost>=0){
                    boolvar x = formula_->newBoolVar("cluster_busy_times_min", r);
                    formula_->addALKWithCheckVar(busies, min, x);
                    pseudoVars_.push_back({x,weight});
                }
                else{
                    formula_->addALK(busies, min);
                }
            }
            if(max < busies.size()){
                if(cost>=0){
                    boolvar x = formula_->newBoolVar("cluster_busy_times_max", r);
                    formula_->addAMKWithCheckVar(busies, max, x);
                    pseudoVars_.push_back({x,weight});
                }
                else{
                    formula_->addAMK(busies, max);
                }
            }
        }

    }
}



SMTFormula * SATBasicModel::encode(int LB, int UB){
    SMTFormula * f = new SMTFormula();
    *f = *formula_;///POSSIBLE ERROR POINT!
    f->addClauses(clauses_);
    std::vector<literal> boolvars;
    std::vector<int> q;
    for(std::pair<boolvar, int> p: pseudoVars_){
        boolvars.push_back(p.first);
        q.push_back(p.second);
    }
    f->addPB(q,boolvars,UB);

    return f;
}

void SATBasicModel::setModel(const EncodedFormula &ef, int lb, int ub, const vector<bool> &bmodel, const vector<int> &imodel){
    xt_Res_ = std::unordered_map<int,std::vector<bool>>();
    xs_Res_ = std::unordered_map<int,std::vector<bool>>();
    xd_Res_ = std::unordered_map<int, std::unordered_map<int,std::vector<bool>>>();
    pseudoVars_Res_ = std::vector<bool>();
    for(auto & it : xt_){
        std::vector<bool> vec;
        for(boolvar b : it.second)
            vec.push_back(ef.f->getBValue(b,bmodel));
        xt_Res_.insert({it.first,vec});
    }
    for(auto & it : xs_){
        std::vector<bool> vec;
        for(boolvar b : it.second)
            vec.push_back(ef.f->getBValue(b,bmodel));
        xs_Res_.insert({it.first, vec});
    }
    for(auto & it: xd_){
        std::unordered_map<int, std::vector<bool>> map_;
        for(auto & it2 : it.second){
            std::vector<bool> vec;
            for(boolvar b : it2.second)
                vec.push_back(ef.f->getBValue(b,bmodel));
            map_.insert({it2.first, vec});
        }
        xd_Res_.insert({it.first, map_});
    }
    int _aux_pseudo_total = 0;
    for (auto p_ : pseudoVars_){
        bool value = ef.f->getBValue(p_.first,bmodel);
        if (value) _aux_pseudo_total += p_.second;
        pseudoVars_Res_.push_back(value);
    }
    pseudoValue = _aux_pseudo_total;
}
int SATBasicModel::getObjective() const{
    return pseudoValue;
//    return 0;
}
bool SATBasicModel::printSolution(ostream &os) const{
    std::vector<AssignmentEntry> result_vec = get_time_assignments();
    //for (AssignmentEntry entry : result_vec)
    //    os<<"Event reference: " << entry.event_id << " --- Duration: " << entry.duration << " --- Time reference: " << entry.time_id<< std::endl;
    std::unordered_map<std::string, std::vector<AssignmentEntry>> week_events;
    std::vector<std::string> week_days;
    for(AssignmentEntry entry : result_vec){
        std::set<std::string> time_groups_ids = times_.find(entry.time_id)->second->get_groups();
        std::string week_day;
        for(std::string time_group_id : time_groups_ids){
            Group* group = time_groups_.find(time_group_id)->second;
            if(group->get_opt() == "Day"){
                week_day = group->get_name();
                break;
            }
        }
        week_events[week_day].push_back(entry);
    }

    for(auto it: week_events){
        std::sort(it.second.begin(), it.second.end(), [](AssignmentEntry a, AssignmentEntry b) -> bool{ return a.time_id<b.time_id;});
        week_days.push_back(it.first);
    }

    std::sort(week_days.begin(), week_days.end(), [this](std::string a, std::string b)->bool{
        auto it1 = week_days_dict.find(a[1]);
        auto it2 = week_days_dict.find(b[1]);
        return it1->first<it2->first;
    });

    std::vector<std::vector<std::string>> schedule_by_columns;
    int longest_day_times=0;
    for(auto it : week_events){
        std::vector<std::string> week;
        week.push_back(it.first);
        for(AssignmentEntry entry : it.second){
            for(int i = 0; i<entry.duration; i++)
                week.push_back(entry.event_id);
        }
        schedule_by_columns.push_back(week);
        if (longest_day_times<week.size()) longest_day_times = week.size();
    }

    TextTable t( '-', '|', '+' );

    for(int i = 0; i<longest_day_times; i++){
        for(int j = 0; j<week_days.size(); j++){
            if(schedule_by_columns[j].size()>i)
                t.add(schedule_by_columns[j][i]);
        }
        t.endOfRow();
    }

    os<<std::endl<<t;

    return true;

}



