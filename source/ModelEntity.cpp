#include "ModelEntity.h"
#include <iostream>


int times_counter = 0;
int events_counter = 0;
int resources_counter = 0;
int resource_type_counter = 0;


Group::Group(const std::string& id, const std::string& name, const std::string& gtype, const std::string& opt){
	identifier_ = id;
	name_ = name;
	gtype_ = gtype;
    opt_ = opt;
	elems_ = std::set<std::string>();
}

void Group::add_element(const std::string& ref){
	elems_.insert(ref);
}

std::set<std::string> Group::get_elems() const{
	return elems_;
}

std::string Group::get_opt() const { return opt_;}

std::string Group::get_name() const { return name_; }

ModelEntity::ModelEntity(const std::string &id, const std::string &name) {
    identifier_ = id;
    name_ = name;
    num_ = -1;
    groups_ = std::set<std::string>();
}

int ModelEntity::get_num() const{
    return num_;
}

std::string ModelEntity::get_identifier() const{
    return identifier_;
}

void ModelEntity::add_group(const std::string& group_ref){
	groups_.insert(group_ref);
}

std::set<std::string> ModelEntity::get_groups() const{ return groups_; }

ResourceType::ResourceType(const std::string &id, const std::string &name) : ModelEntity(id, name) {
    type_ = "resource_type";
    lower_bound_ = upper_bound_ = 0;
    domain_ = std::set<int>();
    num_ = resource_type_counter;
    resource_type_counter++; 
}

void ResourceType::add_resource(const Resource& resource) {
    groups_.insert(resource.get_identifier());
    domain_.insert(resource.get_num());

    //update upper and lower bounds
    if (resource.get_num() > upper_bound_) upper_bound_ = resource.get_num();
    else if(resource.get_num() < lower_bound_) lower_bound_ = resource.get_num();
}

int ResourceType::get_upper_bound() const { return upper_bound_; }

int ResourceType::get_lower_bound() const { return lower_bound_; }


Time::Time(const std::string &id, const std::string &name) : ModelEntity(id, name) {
    type_ = "time";
    num_ = times_counter;
    times_counter++;
}


Resource::Resource(const std::string &id, const std::string &name, std::string r_type) : ModelEntity(id, name) {
	if (r_type.empty())
		throw ModelException("Cannot create a Resource without type");
	rtype_ = r_type;
	type_ = "resources";
	num_ = resources_counter;
	resources_counter++;
}

std::string Resource::get_rtype_ref() const { return rtype_; }

Event::Event(const std::string& id, const std::string& rename, const int &duration, const std::string &color): ModelEntity(id,rename) {
	type_ = "event";
	num_ = events_counter;
	events_counter++;
	duration_ = duration;
	color_ = color;
	time_ = "";
    resources_ = std::unordered_map<std::string, std::shared_ptr<Resource>>();
	mapping_ = std::unordered_map<std::string, std::string>();
	needed_ = std::set<int>();
}


bool Event::has_role(const std::string& role) const{
	return resources_.find(role) != resources_.end();
}

bool Event::has_preassigned_resource(const std::string& role) const {
	return resources_.find(role)->second != nullptr;
}

bool Event::has_preassigned_time() const{	return !time_.empty();  }

bool Event::is_preassigned(int num) const{
	bool result = false;
	for (auto & resource : resources_) if (resource.second->get_num() == num) result = true;
	return result;
}

std::set<std::shared_ptr<Resource>> Event::get_preassigned_resources() const{
    auto result = std::set<std::shared_ptr<Resource>>();
	for (auto& resource : resources_) if (resource.second != nullptr) result.insert(resource.second);
	return result;
}

Resource Event::get_preassigned_resource(const std::string& role){	return *resources_[role];   }

std::set<std::string> Event::get_roles() const{
	auto result = std::set<std::string>();
	for (auto & resource : resources_) result.insert(resource.first);
	return result;
}

std::set<int> Event::get_needed_resources() const{	return needed_; }

int Event::get_duration() const{    return duration_;   }

std::string Event::get_time_ref() const {   return time_;    }


std::unordered_map<std::string, std::shared_ptr<Resource>> Event::get_resources() const{    return resources_;   }

Resource Event::get_preassigned(const int& num) const {
	for (auto & resource : resources_) if (resource.second->get_num() == num) return *resource.second;
}

std::set<int> Event::get_preassigned_nums() const {
	auto result = std::set<int>();
	for(auto & resource : resources_)
		if (resource.second != nullptr) result.insert(resource.second->get_num());
	return result;
}

void Event::set_time(const std::string& time_ref){ time_ = time_ref; }

void Event::attach_reosurce(std::shared_ptr<Resource> resource, std::string role, std::string resource_type, std::shared_ptr<Model> model){
	int num;
	if (resource == nullptr){
		if (role.empty() || resource_type.empty())
			throw ModelException("Cannot attach a resource without role nor resource_type");
		resources_[role] = nullptr;
		mapping_[role] = resource_type;

		num = model->get_rtype_by_ref(resource_type)->get_num();
	}
	else{
		if (resource_type.empty() || resource->get_rtype_ref() == resource_type){
			if (role.empty()){
				//find an unused role name for the resource
				int i = 0;
				role = "role_" + std::to_string(i);
				while (resources_.find(role) != resources_.end()) role = "role_" + std::to_string(++i);
			}
			resources_[role] = resource;
			if (resource_type.empty())
				mapping_[role] = resource->get_rtype_ref();
			else
				mapping_[role] = resource_type;
		}
		else 
			throw ModelException("Malformed input given a wrong resource_type for the preassigned resource");

		num = model->get_rtype_by_ref(resource->get_rtype_ref())->get_num();
	}

	if (needed_.find(num) != needed_.end())
		std::cout << "Warning! Encountered an Event which needs two or more resources of the same type" << std::endl;
	else
		needed_.insert(num);
}







