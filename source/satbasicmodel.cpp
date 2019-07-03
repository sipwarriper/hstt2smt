#include "satbasicmodel.h"
#include "XMLParser.h"
#include <unordered_map>

SATBasicModel::SATBasicModel(std::string filename) : Model(){
    formula_ = new SMTFormula();


    auto parser = XMLParser(this, filename);
    parser.parse_model();


    //we need to remove duplicated clauses
    //TODO: dis

}


void SATBasicModel::on_parsed_events(bool spread_constraint){
    //initialize xr_
    for(auto &r : resources_)
        xr_.insert({r.second->get_num(), std::set<int>()});

    int time_count = times_.size();
    int counter = 0;

    for(auto & event_it : events_){
        counter++;
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
            std::vector<clause> _cls(time_count);
            for(int i = 0; i<time_count; i++) _cls[i] = !xs_[e_num][i] | xt_[e_num][i]; //xs implies xt
            clauses_.insert(clauses_.end(), _cls.begin(), _cls.end());

            //if event is taking place in xt and not in xt-1 then it must be a starting time
            _cls.clear(); _cls.resize(time_count);
            for(int i = 1; i<time_count; i++) _cls[i] = !xt_[e_num][i] | xt_[e_num][i-1] | xs_[e_num][i]; //TODO: es podria afegir un exactly one per apurar aquesta clausula (potser ja hi es)
            clauses_.insert(clauses_.end(), _cls.begin(), _cls.end());

            //if an envent occurs at the first time slot, then it must be a starting time.
            clauses_.push_back(!xt_[e_num][0] | xs_[e_num][0]);

        }
        //xd constraints:
        //      i -> stands for the variables representin the starting times with duration i
        xd_[e_num] = std::unordered_map<int, std::vector<boolvar>>();
        for(int i = 1; i<event->get_duration(); i++){
            std::vector<boolvar> xd_e_i(time_count);
            for (int j=0;j<time_count;j++) xd_e_i[j] = formula_->newBoolVar("xd",e_num,i,j);
            xd_[e_num][i] = xd_e_i;

            vector<clause> _cls(time_count);
            for(int j=0;j<time_count;j++) _cls[j] = !xd_[e_num][i][j] | xs_[e_num][j];

            if(!spread_constraint){
                //time immediately after an event of duration d, must be set to false
                _cls.clear(); _cls.resize(time_count-i);
                for (int j=0; j<time_count-i; j++) _cls[j] = !xd_[e_num][i][j] | !xt_[e_num][j+i];
                clauses_.insert(clauses_.end(), _cls.begin(), _cls.end());

                //also negate all unfeasible times (the laste ones)
                _cls.clear(); _cls.resize(time_count - (time_count-i+1));
                for(int j=time_count-i+1;j<time_count;j++) _cls[j] = !xd_[e_num][i][j];
                clauses_.insert(clauses_.end(), _cls.begin(), _cls.end());
            }
            // when a bit of xd[e][i] make sure it takes palce in i consecutive slots
            _cls.clear();
            for(int t = 0; t<time_count-i+1; i++){
                for(int ti = t; ti<t+i; t++)
                    _cls.push_back(!xd_[e_num][i][t] | xt_[e_num][ti]);
            }
            clauses_.insert(clauses_.end(), _cls.begin(), _cls.end());

            //if an event wit duration d i taking place, make sure no other events start while the event is taking place.
            if(i>1){ //TODO: revisar
                _cls.clear();
                for(int t = 0; t<time_count-i+1; i++){
                    for(int ti = t+1; ti<t+i; t++)
                        _cls.push_back(!xd_[e_num][i][t] | !xs_[e_num][ti]);
                }
                for(int t = 0; t<time_count-i+1; i++){
                    for(int di = 1; di<i; di++){
                        for(int ti = t; ti<ti+i; ti++)
                            _cls.push_back(!xd_[e_num][i][t] | !xd_[e_num][di][ti]);
                    }
                }
                clauses_.insert(clauses_.end(), _cls.begin(), _cls.end());
            }
        }
        //If an event starts at time t, then exactly one Xd must be set
        if(event->get_duration() > 1){
            for(int t = 0; t<time_count; t++){
                std::vector<literal> _lts;
                for(int i=1;i<time_count; i++) _lts.push_back(xd_[e_num][i][t]);
                _lts.push_back(!xs_[e_num][t]);
                formula_->addEO(_lts);
            }
        }
        else{
            std::vector<clause> _cls;
            for(int i=1;i<time_count; i++)
                _cls.push_back(!xs_[e_num][i] | xd_[e_num][1][i]);
            clauses_.insert(clauses_.end(), _cls.begin(), _cls.end());
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
