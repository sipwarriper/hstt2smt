#include "Model.h"

Model::Model()
{
    events_ = std::map<std::string, std::shared_ptr<Event>>();
    resources_ = std::map<std::string, std::shared_ptr<Resource>>();
    times_ = std::map<std::string, std::shared_ptr<Time>>();

    rtypes_ = std::unordered_map<std::string, std::shared_ptr<ResourceType>>();

    time_groups_ = std::unordered_map<std::string, std::shared_ptr<Group>>();
    event_groups_ = std::unordered_map<std::string, std::shared_ptr<Group>>();
    resource_groups_ = std::unordered_map<std::string, std::shared_ptr<Group>>();

    num2event_ = std::unordered_map<int, std::shared_ptr<Event>>();
    num2resource_ = std::unordered_map<int, std::shared_ptr<Resource>>();
    num2time_ = std::unordered_map<int, std::shared_ptr<Time>>();
    num2rtype_ = std::unordered_map<int, std::shared_ptr<ResourceType>>();
}

void Model::register_time(const std::string& id, const std::string& name){
	if (times_.find(id) == times_.end()) {
        std::shared_ptr<Time> time(new Time(id, name));
		times_.insert({id, time});
		num2time_.insert({ time->get_num(),time});
	}
	else throw ModelException("Multiple Times with the same id");
}

void Model::register_event(const std::string& id, const std::string& name, const int &duration, const std::string &color){
	if (events_.find(id) == events_.end()) {
        std::shared_ptr<Event> event(new Event(id, name, duration, color));
		events_.insert({id,event});
        num2event_.insert({ event->get_num(), event });
		auto testing = event->get_preassigned_resources();
	}
	else throw ModelException("Multiple Events with the same id");
}

void Model::register_resource(std::string id, std::string name, std::string rtype_id){
	if(rtypes_.find(rtype_id) != rtypes_.end()){
		if(resources_.find(id) == resources_.end()){
            std::shared_ptr<Resource> resource(new Resource(id, name, rtype_id));
			resources_.insert({id, resource});
			num2resource_.insert({ resource->get_num(), resource });
			rtypes_[rtype_id]->add_resource(*resource);
		}
		else throw ModelException("Multiple Resources with the same id");
	}
	else throw ModelException("Attempt to register a resource with an unregistered resource type");
}

void Model::register_resource_type(std::string id, std::string name){
	if(rtypes_.find(id) == rtypes_.end()){
        std::shared_ptr<ResourceType> rtype(new ResourceType(id, name));
		rtypes_.insert({ id, rtype });
	}
}

std::shared_ptr<Time> Model::get_time_by_ref(std::string ref) const{
	auto it = times_.find(ref);
	if (it != times_.end()) return it->second;
	return nullptr;
}

std::shared_ptr<Resource> Model::get_resource_by_ref(std::string ref) const{
	auto it = resources_.find(ref);
	if (it != resources_.end()) return it->second;
	return nullptr;
}

std::shared_ptr<Event> Model::get_event_by_ref(std::string ref) const{
	auto it = events_.find(ref);
	if (it != events_.end()) return it->second;
	return nullptr;
}

std::shared_ptr<ResourceType> Model::get_rtype_by_ref(std::string ref) const{
	auto it = rtypes_.find(ref);
	if (it != rtypes_.end()) return it->second;
	return nullptr;
}

void Model::declare_time_group(const std::string& id, const std::string& name, const std::string& tag){
	if (time_groups_.find(id)==time_groups_.end()){
        std::shared_ptr<Group> group(new Group(id, name, "TimeGroup", tag));
		time_groups_.insert({id, group});
	}
	else throw ModelException("Already defined TimeGroup");
}

void Model::declare_resource_group(const std::string& id, const std::string& name, const std::string& rtype_ref){
	if(rtypes_.find(rtype_ref) != rtypes_.end()){
		if(resource_groups_.find(id) == resource_groups_.end()){
            std::shared_ptr<Group> group(new Group(id, name, "ResourceGroup", rtype_ref));
			resource_groups_.insert({ id, group });
		}
		else throw ModelException("Already defined ResourceGroup");
	}
	else throw ModelException("Undeclared resource type");
}

void Model::declare_event_group(const std::string& id, const std::string& name, const std::string& tag){
	if (event_groups_.find(id) == event_groups_.end()){
        std::shared_ptr<Group> group(new Group(id, name, "EventGroup", tag));
		event_groups_.insert({ id, group });
	}
	else throw ModelException("Already defined EventGroup");
}

void Model::time_to_group(const std::string& time_id, const std::string& group_ref){
	if (times_.find(time_id) == times_.end() || time_groups_.find(group_ref) == time_groups_.end())
		throw ModelException("Undeclared reference encountered");
	times_[time_id]->add_group(group_ref);
	time_groups_[group_ref]->add_element(time_id);
}

void Model::resource_to_group(const std::string& resource_id, const std::string& group_ref){
	if (resources_.find(resource_id) == resources_.end() || resource_groups_.find(group_ref) == resource_groups_.end())
		throw ModelException("Undeclared reference encountered");
	if (resources_[resource_id]->get_rtype_ref() != resource_groups_[group_ref]->get_opt())
		throw ModelException("Attached resource to a group of different type");
	resources_[resource_id]->add_group(group_ref);
    resource_groups_[group_ref]->add_element(resource_id);
}

void Model::event_to_group(const std::string& event_id, const std::string& group_ref){
	if (events_.find(event_id) == events_.end() || event_groups_.find(group_ref) == event_groups_.end())
		throw ModelException("Undeclared reference encountered");
	events_[event_id]->add_group(group_ref);
	event_groups_[group_ref]->add_element(event_id);
}

int Model::upper_bound_by_resource_type(const int &num) const{
	return num2rtype_.find(num)->second->get_upper_bound();
}

std::set<std::string> Model::get_events_from_group(const std::string& group_ref) const{
	if (group_ref.empty() || event_groups_.find(group_ref) == event_groups_.end())
		throw ModelException("Incorrect group_ref");
	return event_groups_.find(group_ref)->second->get_elems();
}

std::set<std::string> Model::get_resources_from_group(const std::string& group_ref) const{
	if (group_ref.empty() || resource_groups_.find(group_ref) == resource_groups_.end())
		throw ModelException("Incorrect group_ref");
	return resource_groups_.find(group_ref)->second->get_elems();
}

std::set<std::string> Model::get_times_from_group(const std::string& group_ref) const{
	if (group_ref.empty() || time_groups_.find(group_ref) == time_groups_.end())
		throw ModelException("Incorrect group_ref");
	return time_groups_.find(group_ref)->second->get_elems();
}




