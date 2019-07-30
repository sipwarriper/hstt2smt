#pragma once

#include <string>
#include <set>
#include <unordered_map>
#include "Model.h"

//global variables

extern int times_counter;
extern int events_counter;
extern int resources_counter;
extern int resource_type_counter;

class Model;

class Group{
public:
	Group(const std::string &id="", const std::string &name="", const std::string &gtype="", const std::string &opt = "");
	void add_element(const std::string & ref);
	std::set<std::string> get_elems() const;
	std::string get_opt() const;
    std::string get_name() const;

protected:
	std::string identifier_;
	std::string name_;
	std::string gtype_; // group type: {TimeGroup, EventGroup, ResourceGroup}
	std::string opt_; //  optional info about group (whether it represents a Day, Week, Course..)
	std::set<std::string> elems_; // set with the ids of elements forming this group.

};

class ModelEntity{
public:
    ModelEntity(const std::string &id = "", const std::string &name ="NAMELESS");
    int get_num() const;
    std::string get_identifier() const;
	void add_group(const std::string& group_ref);
    std::set<std::string> get_groups() const;

protected:
    std::string identifier_;
    std::string name_;
    int num_;
    std::set<std::string> groups_;

    //TODO: cal revisar tipus de groups.

};

class Resource;
class ResourceType: public ModelEntity{
public:
    ResourceType(const std::string &id="", const std::string &name ="NAMELESS_RESOURCE_TYPE");
    void add_resource(const Resource& resource);
	int get_upper_bound() const;
	int get_lower_bound() const;

protected:
    std::string type_;
    int lower_bound_, upper_bound_;
    std::set<int> domain_;


};

class Time: public ModelEntity{
public:
    Time(const std::string &id="", const std::string &name="NAMELESS_TIME");

private:
    std::string type_;
};

class Resource: public ModelEntity{
public:
	Resource(const std::string &id="", const std::string &name = "NAMELESS_TIME", std::string r_type = "");

	std::string get_rtype_ref() const;

protected:
    std::string type_;
	std::string rtype_; //resource type identifier 

};

class Event: public ModelEntity{
public:
	Event(const std::string& id="", const std::string& name = "NAMELESS_EVENT", const int &duration = 1, const std::string &color= "");
	bool has_role(const std::string& role) const;
	bool has_preassigned_resource(const std::string& role) const;
	bool has_preassigned_time() const;
	bool is_preassigned(int num) const;

	std::set<const Resource*> get_preassigned_resources() const;
	Resource get_preassigned_resource(const std::string &role);

	std::set<std::string> get_roles() const;
	std::set<int> get_needed_resources() const;
    int get_duration() const;
    std::string get_time_ref() const;
    std::unordered_map<std::string, Resource*> get_resources() const;

	Resource get_preassigned(const int &num) const;
	std::set<int> get_preassigned_nums() const; //returns a set containing all numerals of preassigned resources

	void set_time(const std::string& time_ref);
	void attach_reosurce(Resource* resource, std::string role, std::string resource_type, Model* model);
	
protected:
	std::string type_;
	std::string color_;
	int duration_;
	std::string time_; //time reference

	std::unordered_map<std::string, Resource*> resources_;
	std::unordered_map<std::string, std::string> mapping_; //roles->rtype_ref

	std::set<int> needed_; //set of needed ResourceType num's 

};
