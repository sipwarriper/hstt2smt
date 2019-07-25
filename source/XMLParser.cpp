#include "XMLParser.h"
#include <iostream>
#include <limits>
#include <algorithm>
#include <math.h>
#include <limits.h>

XMLParser::XMLParser(Model* instance, std::string filename) {

    if(!document.load_file(filename.c_str())){
        throw std::runtime_error("Error loading xml file. Aborting.");
    }
    model_ = instance;

	xml_source_filename_ = filename;
	xml_root_ = document.root();

	assign_time_ = std::set<std::string>();
	assign_res_ =  std::unordered_map<std::string, std::set<std::string>*>();
	xml_instances_ = xml_root_.child("HighSchoolTimetableArchive").child("Instances");

}

void XMLParser::parse_model(){
	xml_first_instance_ = xml_instances_.first_child();
	parse_instance(xml_first_instance_);
}

void XMLParser::parse_instance(const pugi::xml_node &xml_instance){
	if (!xml_instance) throw xmlParserException();

	parse_times(xml_instance.child("Times"));

	parse_resources(xml_instance.child("Resources"));

	parse_events(xml_instance.child("Events"));

	parse_constraints();
}

void XMLParser::parse_times(const pugi::xml_node &xml_times){
	if (!xml_times) throw xmlParserException();

	parse_time_groups(xml_times.child("TimeGroups"));

	for (pugi::xml_node xml_elem : xml_times.children("Time")){
		//get id
		std::string id = xml_elem.attribute("Id").as_string();
		//register time elment in the model
		model_->register_time(id, xml_elem.child("Name").text().as_string());

		pugi::xml_node xml_week = xml_elem.child("Week");
		if (xml_week)
			model_->time_to_group(id, xml_week.attribute("Reference").as_string());

		pugi::xml_node xml_day = xml_elem.child("Day");
		if (xml_day)
			model_->time_to_group(id, xml_day.attribute("Reference").as_string());

		pugi::xml_node xml_groups = xml_elem.child("TimeGroups");
		if (xml_groups){
			for (pugi::xml_node xml_group : xml_groups.children("TimeGroup"))
				model_->time_to_group(id, xml_group.attribute("Reference").as_string());
		}
	}
}

void XMLParser::parse_resources(const pugi::xml_node& xml_resources){
	if (!xml_resources) throw xmlParserException();
	parse_resource_types(xml_resources.child("ResourceTypes"));
	parse_resource_groups(xml_resources.child("ResourceGroups"));

	//Iterate over all <Resource> tags, updating each one into our given model
	for(pugi::xml_node xml_elem : xml_resources.children("Resource")){
		std::string id = xml_elem.attribute("Id").as_string();
		std::string name = xml_elem.child("Name").text().as_string();
		std::string rtype = xml_elem.child("ResourceType").attribute("Reference").as_string();
		
		model_->register_resource(id, name, rtype);
		//parse the groups this elem belongs to
        pugi::xml_node xml_groups = xml_elem.child("ResourceGroups");
		if(xml_groups){
            for (pugi::xml_node xml_group : xml_groups.children("ResourceGroup"))
				model_->resource_to_group(id, xml_group.attribute("Reference").as_string());
		}
	}
}

void XMLParser::parse_events(const pugi::xml_node& xml_events){
	if (!xml_events) throw xmlParserException();
	
	parse_event_groups(xml_events.child("EventGroups"));

	//pre-parse all events (just fill event groups)
	for(pugi::xml_node xml_elem : xml_events.children("Event")){
		std::string id = xml_elem.attribute("Id").as_string();
		std::string color = xml_elem.attribute("Color").as_string();
		std::string name = xml_elem.child("Name").text().as_string();
		int duration = xml_elem.child("Duration").text().as_int();

		//register the event in the model
		model_->register_event(id, name, duration, color);

		//Parse Event groups attached to the event
		pugi::xml_node xml_course = xml_elem.child("Course");
		if (xml_course)
			model_->event_to_group(id, xml_course.attribute("Reference").as_string());
		pugi::xml_node xml_groups = xml_elem.child("EventGroups");
		if(xml_groups){
			for (pugi::xml_node xml_group : xml_groups.children("EventGroup"))
				model_->event_to_group(id, xml_group.attribute("Reference").as_string());
		}
	}
	//parse all assign-like constraints in order to be able to filter incoming events
	pugi::xml_node xml_constraints = xml_first_instance_.child("Constraints");
	auto events = std::set<std::string>();
	for (pugi::xml_node xml_constraint : xml_constraints.children("AssignTimeConstraint")) {
		auto res = parse_assign_time_constraint(xml_constraint);
		events.insert(res.begin(), res.end());
	}
	assign_time_ = events;
	for (pugi::xml_node xml_constraint : xml_constraints.children("AssignResourceConstraint"))
		parse_assign_resource_constraint(xml_constraint);

	//Iterate over all <Event> tags, updating each one into our given Model
	for (pugi::xml_node xml_elem : xml_events.children("Event")){
		std::string id = xml_elem.attribute("Id").as_string();

		Event* event = model_->get_event_by_ref(id);
		pugi::xml_node xml_resources = xml_elem.child("Resources");
		
		if (!xml_resources || !xml_resources.child("Resource"))
		    throw std::runtime_error("Unsupported feature: Encountered an Event without any resource requirements");

		for(pugi::xml_node xml_resource : xml_resources.children("Resource")){
			std::string ref = xml_resource.attribute("Reference").as_string();
			Resource* resource = model_->get_resource_by_ref(ref);
			std::string role = "";
			std::string rtype = "";

			if (pugi::xml_node xml_role = xml_resource.child("Role"))
				role = xml_role.text().as_string();
			if (pugi::xml_node xml_rtype = xml_resource.child("ResourceType"))
				rtype = xml_rtype.attribute("Reference").as_string();

			//If event has no preassigned resource for a given role, check that it is intended to be filled
			bool id_in_assign_res = assign_res_.find(id) != assign_res_.end();
			bool role_in_id_assign_res = id_in_assign_res && assign_res_[id]->find(role) != assign_res_[id]->end();
			if (ref.empty() && (!id_in_assign_res || !role_in_id_assign_res))
				std::cout << "Warning: role " << role << " for event " << id << " is not intended to be filled by the model" << std::endl;
			else
				event->attach_reosurce(resource, role, rtype, model_);

			//Check if the event is expected to attend a preassigned time
			pugi::xml_node xml_time = xml_elem.child("Time");
			if (xml_time) {
				std::string time_ref = xml_time.attribute("Reference").as_string();
				if (model_->get_time_by_ref(time_ref) == nullptr) throw std::runtime_error("Event with an undeclared Time reference");
				event->set_time(time_ref);
			}
			else if (assign_time_.find(id) == assign_time_.end())
				throw std::runtime_error("Event without Time nor assign time constraint");
		}
		//notify the model that all events have been fully parsed (before any constraints ofc)
		model_->on_parsed_events(xml_constraints.child("SpreadEventsConstraint"));

		//todo preguntar an josep lo del parse_optimize_events();
		//parse_optimize_events();
	}
}

void XMLParser::parse_time_groups(const pugi::xml_node &xml_tgroups){
	if (xml_tgroups){ //Optional
		for (pugi::xml_node xml_group : xml_tgroups.children("Week"))
			model_->declare_time_group(xml_group.attribute("Id").as_string(), xml_group.child("Name").text().as_string(), "Week");

		for (pugi::xml_node xml_group : xml_tgroups.children("Day"))
			model_->declare_time_group(xml_group.attribute("Id").as_string(), xml_group.child("Name").text().as_string(), "Day");

		for (pugi::xml_node xml_group : xml_tgroups.children("TimeGroup"))
			model_->declare_time_group(xml_group.attribute("Id").as_string(), xml_group.child("Name").text().as_string());
	}
}

void XMLParser::parse_resource_groups(const pugi::xml_node& xml_rgroups){
	if(xml_rgroups){
		for (pugi::xml_node xml_rgroup : xml_rgroups) {
			std::string id = xml_rgroup.attribute("Id").as_string();
			std::string name = xml_rgroup.child("Name").text().as_string();
			std::string rtype = xml_rgroup.child("ResourceType").attribute("Reference").as_string();
			model_->declare_resource_group(id, name, rtype);
		}
	}
}

void XMLParser::parse_event_groups(const pugi::xml_node& xml_egroups){
	if(xml_egroups){
		for (pugi::xml_node xml_course : xml_egroups.children("Course"))
			model_->declare_event_group(xml_course.attribute("Id").as_string(),
				xml_course.child("Name").text().as_string(), "Course");
		for (pugi::xml_node xml_egroup: xml_egroups.children("EventGroup"))
			model_->declare_event_group(xml_egroup.attribute("Id").as_string(),
				xml_egroup.child("Name").text().as_string(), "Course");
	}
}

void XMLParser::parse_resource_types(const pugi::xml_node& xml_resources) {
	if (!xml_resources) std::cout << "warning, given XHSTT has no resource types in it" << std::endl;
	else{
		for (pugi::xml_node xml_rtype : xml_resources.children("ResourceType"))
			model_->register_resource_type(xml_rtype.attribute("Id").as_string(), xml_rtype.child("Name").text().as_string());
	}
}

std::set<std::string> XMLParser::parse_assign_time_constraint(const pugi::xml_node& xml_node){
	auto events = std::set<std::string>();
	if(xml_node && constraint_required(xml_node)){
		pugi::xml_node xml_apply = xml_node.child("AppliesTo");

		events = gather_events_from_constraint(xml_apply);
	}
	//return all involved events
	return events;
}

void XMLParser::parse_assign_resource_constraint(const pugi::xml_node& xml_node){
	if (xml_node && constraint_required(xml_node)) {
		pugi::xml_node xml_apply = xml_node.child("AppliesTo");

		auto events = gather_events_from_constraint(xml_apply);
		std::string role = xml_node.child("Role").text().as_string();

		// Once retrieved all involved events and the role, annotate it on a private variable
		for(std::string event : events){
			if (assign_res_.find(event) == assign_res_.end())
				assign_res_.insert({event, new std::set<std::string>()});
			assign_res_[event]->insert(role);
		}
	}
}

bool XMLParser::constraint_required(const pugi::xml_node& xml_constraint) {
	return xml_constraint.child("Required").text().as_bool();
}

std::set<std::string> XMLParser::gather_events_from_constraint(const pugi::xml_node &xml_apply) const
{
	//Gather all events to which this constraint applies
	auto events = std::set<std::string>();
	pugi::xml_node xml_groups = xml_apply.child("EventGroups");
	if (xml_groups) {
		for (pugi::xml_node xml_group : xml_groups.children("EventGroup")) {
			std::set<std::string> temp = model_->get_events_from_group(xml_group.attribute("Reference").as_string());
			events.insert(temp.begin(), temp.end());
		}
	}
	pugi::xml_node xml_events = xml_apply.child("Events");
	if (xml_events) {
		for (pugi::xml_node xml_event : xml_events.children("Event"))
			events.insert(xml_event.attribute("Reference").as_string());
	}
	return events;
}

std::set<std::string> XMLParser::gather_resources_from_constraint(const pugi::xml_node &xml_apply) const
{
	auto resources = std::set<std::string>();
	pugi::xml_node xml_groups = xml_apply.child("ResourceGroups");
	if (xml_groups) {
		for (pugi::xml_node xml_group : xml_groups.children("ResourceGroup")) {
                std::string id = xml_group.attribute("Reference").as_string();
                std::set<std::string> temp = model_->get_resources_from_group(id);
                resources.insert(temp.begin(), temp.end());
		}
	}
	pugi::xml_node xml_resources = xml_apply.child("Resources");
	if (xml_resources) {
		for (pugi::xml_node xml_resource : xml_resources.children("Resource"))
			resources.insert(xml_resource.attribute("Reference").as_string());
	}
	return resources;
}

std::set<std::string> XMLParser::gather_times_from_constraint(const pugi::xml_node& xml_apply) const{
	auto times = std::set<std::string>();
	pugi::xml_node xml_groups = xml_apply.child("TimeGroups");
	if (xml_groups) {
		for (pugi::xml_node xml_group : xml_groups.children("TimeGroup")) {
			std::set<std::string> temp = model_->get_times_from_group(xml_group.attribute("Reference").as_string());
			times.insert(temp.begin(), temp.end());
		}
	}
	pugi::xml_node xml_times = xml_apply.child("Times");
	if (xml_times) {
		for (pugi::xml_node xml_time : xml_times.children("Time"))
			times.insert(xml_time.attribute("Reference").as_string());
	}

	return times;
}

int XMLParser::constraint_cost(pugi::xml_node xml_node, int points_of_application){//todo revisar tipo retorn
	bool required = xml_node.child("Required").text().as_bool();
	if (required)
        return -1;

	int weight = xml_node.child("Weight").text().as_int();
	std::string costFunc = xml_node.child("CostFunction").text().as_string();
	std::transform(costFunc.begin(), costFunc.end(), costFunc.begin(), ::tolower);
	if (costFunc == "sum" || costFunc == "linear")
		return weight * points_of_application;
	if (costFunc == "quadratic")
		return weight * static_cast<int>(pow(points_of_application, 2));
	if (costFunc == "step")
		return weight * int(points_of_application > 0);
	std::cout << "unrecognized cost function: " << xml_node.child("CostFunction").text().as_string()<<std::endl;
	throw xmlParserException();
}

void XMLParser::parse_prefer_resources_constraint(const pugi::xml_node& xml_node){
	if(xml_node && constraint_required(xml_node)){
		pugi::xml_node xml_apply = xml_node.child("AppliesTo");

		std::set<std::string> events = gather_events_from_constraint(xml_apply);

		//Gather all resources to which this constraint refers
		std::set<std::string> resources = gather_resources_from_constraint(xml_node);
		std::string role = xml_node.child("Role").text().as_string();
		model_->prefer_resources_constraint(events,resources,role);
	}
}

void XMLParser::parse_prefer_times_constraint(const pugi::xml_node& xml_node){
	if(xml_node){
		pugi::xml_node xml_apply = xml_node.child("AppliesTo");

		std::set<std::string> events = gather_events_from_constraint(xml_apply);

	   //Gather all times to which this constraint refers

		std::set<std::string> times = gather_times_from_constraint(xml_node);

		//Gather duration of the events to which this constraint applies
		pugi::xml_node xml_duration = xml_node.child("Duration");
		int duration = -1;
		if (xml_duration)
			duration = xml_duration.text().as_int();

		int points_of_app = 0;
		for (std::string event : events)
			if (!model_->get_event_by_ref(event)->has_preassigned_time()) points_of_app++;
		int cost = constraint_cost(xml_node, points_of_app);

		//propagate the parsed info to the model
		model_->prefer_times_constraint(cost, events, times, duration);
	}
}

void XMLParser::parse_avoid_clashes_constraint(const pugi::xml_node& xml_node){
	if(xml_node){
		pugi::xml_node xml_apply = xml_node.child("AppliesTo");

		std::set<std::string> resources = gather_resources_from_constraint(xml_apply);

		int points_of_app = resources.size();
		int cost = constraint_cost(xml_node, points_of_app);

		//propagate the parsed info to the model
		model_->avoid_clashes_constraint(cost, resources);
	}
}

void XMLParser::parse_split_events_constraint(const pugi::xml_node& xml_node){
	if(xml_node){
		pugi::xml_node xml_apply = xml_node.child("AppliesTo");
		std::set<std::string> events = gather_events_from_constraint(xml_apply);

		int min = xml_node.child("MinimumDuration").text().as_int();
		int max = xml_node.child("MaximumDuration").text().as_int();
		int min_amount = xml_node.child("MinimumAmount").text().as_int();
		int max_amount = xml_node.child("MaximumAmount").text().as_int();

		//check integrity
		if (max < min || max_amount < min_amount || events.empty())
			throw std::runtime_error("Failed integrity check of split events constraint");

		int points_of_app = events.size();
		int cost = constraint_cost(xml_node, points_of_app);

		model_->split_events_constraint(cost, events, min, max, min_amount, max_amount);
	}
}

void XMLParser::parse_spread_events_constraint(const pugi::xml_node& xml_node){
	if(xml_node){
		//gather all event_groups to which this constraint applies
		auto events_groups = std::set<std::string>();
		pugi::xml_node xml_apply = xml_node.child("AppliesTo");
		pugi::xml_node xml_groups = xml_apply.child("EventGroups");
		if (xml_groups) {
            for (pugi::xml_node xml_group : xml_groups.children("EventGroup")) {
                events_groups.insert(xml_group.attribute("Reference").as_string());
			}
		}

		//gather all time groups to which this constraint applies
		auto time_groups = std::unordered_map<std::string, std::pair<int, int>>();
		pugi::xml_node xml_time_groups = xml_node.child("TimeGroups");
		for(pugi::xml_node xml_time_group : xml_time_groups.children("TimeGroup")){
			std::string ref = xml_time_group.attribute("Reference").as_string();
			int min = xml_time_group.child("Minimum").text().as_int();
			int max = xml_time_group.child("Maximum").text().as_int();

			time_groups.insert({ ref, {min,max} });
		}

		int points_of_app = events_groups.size();
		int cost = constraint_cost(xml_node, points_of_app);

		model_->spread_events_constraint(cost, events_groups, time_groups);
	}
}

void XMLParser::parse_avoid_unavailable_times_constraint(const pugi::xml_node& xml_node){
	if(xml_node){
		pugi::xml_node xml_apply = xml_node.child("AppliesTo");
        std::set<std::string> resources = gather_resources_from_constraint(xml_apply);
		std::set<std::string> times = gather_times_from_constraint(xml_node);

		int points_of_app = resources.size();
		int cost = constraint_cost(xml_node, points_of_app);

		model_->avoid_unavailable_times_constraint(cost, resources, times);
	}
}

void XMLParser::parse_distribute_split_events_constraint(const pugi::xml_node& xml_node){
	if(xml_node){
		pugi::xml_node xml_apply = xml_node.child("AppliesTo");
		const std::set<std::string> events = gather_events_from_constraint(xml_apply);
		int duration = xml_node.child("Duration").text().as_int();
		int min = xml_node.child("Minimum").text().as_int();
		int max = xml_node.child("Maximum").text().as_int();
		//integrity check
		if (max < min || events.empty())
			throw std::runtime_error("Failed integrity check of distribute split events constraint");

		int points_of_app = events.size();
		int cost = constraint_cost(xml_node, points_of_app);
		model_->distribute_split_events_constraints(cost, events, duration, min, max);

	}
}

void XMLParser::parse_limit_idle_times_constraint(const pugi::xml_node& xml_node){
	if(xml_node){
		pugi::xml_node xml_apply = xml_node.child("AppliesTo");
		std::set<std::string> resources = gather_resources_from_constraint(xml_apply);

		auto time_groups = std::set<std::string>();
		for (pugi::xml_node xml_time_group : xml_node.child("TimeGroups").children("TimeGroup"))
			time_groups.insert(xml_time_group.attribute("Reference").as_string());

		int min = xml_node.child("Minimum").text().as_int();
		int max = xml_node.child("Maximum").text().as_int();
		int points_of_app = resources.size();
		int cost = constraint_cost(xml_node, points_of_app);

		model_->limit_idle_times_constraint(cost, resources, time_groups, min, max);
	}
}

void XMLParser::parse_cluster_busy_times_constraint(const pugi::xml_node& xml_node){
	if(xml_node){
		pugi::xml_node xml_apply = xml_node.child("AppliesTo");
		std::set<std::string> resources = gather_resources_from_constraint(xml_apply); 
		
		auto time_groups = std::set<std::string>();
		for (pugi::xml_node xml_time_group : xml_node.child("TimeGroups").children("TimeGroup"))
			time_groups.insert(xml_time_group.attribute("Reference").as_string());

		int min = xml_node.child("Minimum").text().as_int();
		int max = xml_node.child("Maximum").text().as_int();
		int points_of_app = resources.size();
		int cost = constraint_cost(xml_node, points_of_app);

		model_->cluster_busy_times_constraint(cost, resources, time_groups, min, max);
	}
}

void XMLParser::parse_constraints(){
	pugi::xml_node xml_constraints = xml_first_instance_.child("Constraints");

	for (pugi::xml_node xml_constraint : xml_constraints.children("PreferResourcesConstraint"))
		parse_prefer_resources_constraint(xml_constraint);

	for (pugi::xml_node xml_constraint : xml_constraints.children("AvoidClashesConstraint"))
		parse_avoid_clashes_constraint(xml_constraint);

	for (pugi::xml_node xml_constraint : xml_constraints.children("SplitEventsConstraint"))
		parse_split_events_constraint(xml_constraint);

	for (pugi::xml_node xml_constraint : xml_constraints.children("SpreadEventsConstraint"))
		parse_spread_events_constraint(xml_constraint);

	for (pugi::xml_node xml_constraint : xml_constraints.children("PreferTimesConstraint"))
		parse_prefer_times_constraint(xml_constraint);

	for (pugi::xml_node xml_constraint : xml_constraints.children("AvoidUnavailableTimesConstraint"))
		parse_avoid_unavailable_times_constraint(xml_constraint);

	for (pugi::xml_node xml_constraint : xml_constraints.children("DistributeSplitEventsConstraint"))
		parse_distribute_split_events_constraint(xml_constraint);
	
	for (pugi::xml_node xml_constraint : xml_constraints.children("LimitIdleTimesConstraint"))
		parse_limit_idle_times_constraint(xml_constraint);
	
	for (pugi::xml_node xml_constraint : xml_constraints.children("ClusterBusyTimesConstraint"))
		parse_cluster_busy_times_constraint(xml_constraint);

}
















