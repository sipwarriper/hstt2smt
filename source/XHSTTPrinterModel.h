#pragma once

#include "Model.h"

class XHSTTPrinterModel: public Model {
public: 
	XHSTTPrinterModel(std::string filename);
	void register_time(const std::string& id, const std::string& name);
	void register_event(const std::string& id, const std::string& name, const int &duration, const std::string &color = "");
	void register_resource(std::string id, std::string name, std::string rtype);
	void register_resource_type(std::string id, std::string name);

	void set_time_for_event(int event_num, int duration, Time* start_t) override;
	void prefer_resources_constraint(const std::set<std::string> &events_ids, const std::set<std::string> &resources_ids, const std::string &role) override;
	void prefer_times_constraint(const int &cost, const std::set<std::string> &events_ids, const std::set<std::string> &times_ids, int duration) override;
	void avoid_clashes_constraint(const int &cost, const std::set<std::string> &resources_ids) override;
	void split_events_constraint(const int &cost, const std::set<std::string> &events, const int &min, const int &max, const int &min_amount, const int &max_amount) override;
	void spread_events_constraint(const int &cost, const std::set<std::string> &event_groups, std::unordered_map<std::string, std::pair<int, int>> time_groups) override;
	//time_groups is a dictionary of tuples. where keys are time_group references and values are tuples of the kind(minimum, maximum)

	void avoid_unavailable_times_constraint(const int &cost, const std::set<std::string> &resources_ids, const std::set<std::string> &times_ids) override;
	void distribute_split_events_constraints(const int &cost, const std::set<std::string> &event_ids, const int &duration, const int &min, const int &max) override;
	void limit_idle_times_constraint(const int &cost, const std::set<std::string> &resources_ids, const std::set<std::string> &time_groups_ids, const int &min, const int &max) override;
	void cluster_busy_times_constraint(const int &cost, const std::set<std::string> &resources_ids, const std::set<std::string> &time_groups_ids, const int &min, const int &max) override;
	void on_parsed_events(bool spread_constraint = true) override;
};


