#pragma once

#include <map>
#include <unordered_map>
#include <exception>
#include "ModelEntity.h"

class ModelEntity;
class Resource;
class Time;
class Event;
class ResourceType;
class Group;

class Model {

public:
	virtual ~Model() = default;
	Model();
	//TODO: desfer virtuals dels registres
	virtual void register_time(const std::string& id, const std::string& name);
	virtual void register_event(const std::string& id, const std::string& name, const int &duration, const std::string &color = "");
	virtual void register_resource(std::string id, std::string name, std::string rtype);
	virtual void register_resource_type(std::string id, std::string name);

	Time* get_time_by_ref(std::string ref) const;
	Resource* get_resource_by_ref(std::string ref) const;
	Event* get_event_by_ref(std::string ref) const;
	ResourceType* get_rtype_by_ref(std::string ref) const;

	void declare_time_group(const std::string& id, const std::string& name, const std::string& tag ="");
	void declare_resource_group(const std::string& id, const std::string& name, const std::string& rtype_ref);
	void declare_event_group(const std::string& id, const std::string& name, const std::string& tag = "");

	void time_to_group(const std::string& time_id, const std::string& group_ref);
	void resource_to_group(const std::string& resource_id, const std::string& group_ref);
	void event_to_group(const std::string& event_id, const std::string& group_ref);

	int upper_bound_by_resource_type(const int &num) const; //Function used to determine the upper bound of a variable representing a given resource type

	std::set<std::string> get_events_from_group(const std::string & group_ref) const;
	std::set<std::string> get_resources_from_group(const std::string & group_ref) const;
	std::set<std::string> get_times_from_group(const std::string & group_ref) const;

	virtual void set_time_for_event(int event_num, int duration, Time* start_t) = 0;
	virtual void prefer_resources_constraint(const std::set<std::string> &events_ids, const std::set<std::string> &resources_ids, const std::string &role) = 0;
	virtual void prefer_times_constraint(const int &cost, const std::set<std::string> &events_ids, const std::set<std::string> &times_ids, int duration) = 0;
	virtual void avoid_clashes_constraint(const int &cost, const std::set<std::string> &resources_ids) = 0;
	virtual void split_events_constraint(const int &cost, const std::set<std::string> &events, const int &min, const int &max, const int &min_amount, const int &max_amount) = 0;
	virtual void spread_events_constraint(const int &cost, const std::set<std::string> &event_groups, std::unordered_map<std::string, std::pair<int, int>> time_groups) = 0;
	//time_groups is a dictionary of tuples. where keys are time_group references and values are tuples of the kind(minimum, maximum)

	virtual void avoid_unavailable_times_constraint(const int &cost, const std::set<std::string> &resources_ids, const std::set<std::string> &times_ids) = 0;
	virtual void distribute_split_events_constraints(const int &cost, const std::set<std::string> &event_ids, const int &duration, const int &min, const int &max) = 0;
	virtual void limit_idle_times_constraint(const int &cost, const std::set<std::string> &resources_ids, const std::set<std::string> &time_groups_ids, const int &min, const int &max) = 0;
	virtual void cluster_busy_times_constraint(const int &cost, const std::set<std::string> &resources_ids, const std::set<std::string> &time_groups_ids, const int &min, const int &max) = 0;
	virtual void on_parsed_events(bool spread_constraint=true) = 0;
	//TODO: cal ficar parametres i arreglar el return d'aquests metodes

	//todo: virtual std::vector<> get_times_assignments() = 0;
 


protected:
	std::map<std::string, Event*> events_;
	std::map<std::string, Resource*> resources_;
	std::map<std::string, Time*> times_;

	std::unordered_map<std::string, ResourceType*> rtypes_;

	std::unordered_map<std::string, Group*> time_groups_;
	std::unordered_map<std::string, Group*> event_groups_;
	std::unordered_map<std::string, Group*> resource_groups_;


	//num2 dictionaries (allowing fast fetching of elements by its elements.num attribute)
	std::unordered_map<int, Event*> num2event_;
	std::unordered_map<int, Resource*> num2resource_;
	std::unordered_map<int, Time*> num2time_;
	std::unordered_map<int, ResourceType*> num2rtype_;
	   	 
};

class ModelException : public std::exception{
private:
	std::string msg_;
public:
	ModelException(const std::string & msg){
		msg_ = msg;
	}
	virtual const char* what() const throw(){
		return msg_.c_str();
	}
};
