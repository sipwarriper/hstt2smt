#pragma once

#include <pugixml.hpp>
#include <exception>
#include "Model.h"

class XMLParser {
public:
    XMLParser(Model* instance, std::string filename); //constructor


    void parse_model();

private:
    pugi::xml_document document;
    Model *model_;
	pugi::xml_node xml_root_;
	std::string xml_source_filename_;
	pugi::xml_node xml_instances_;
	pugi::xml_node xml_first_instance_;

	std::set<std::string> assign_time_;
	std::unordered_map<std::string, std::set<std::string>*> assign_res_;

	void parse_instance(const pugi::xml_node &xml_instance);

	void parse_times(const pugi::xml_node &xml_times);
	void parse_resources(const pugi::xml_node &xml_resources);
	void parse_events(const pugi::xml_node &xml_events);

	void parse_time_groups(const pugi::xml_node &xml_tgroups);
	void parse_resource_groups(const pugi::xml_node &xml_rgroups);
	void parse_event_groups(const pugi::xml_node &xml_egroups);
	void parse_resource_types(const pugi::xml_node &xml_resources);

	std::set<std::string> parse_assign_time_constraint(const pugi::xml_node &xml_node);
	void parse_assign_resource_constraint(const pugi::xml_node &xml_node);

    static bool constraint_required(const pugi::xml_node &xml_constraint);

	std::set<std::string> gather_events_from_constraint(const pugi::xml_node &xml_apply) const;
	std::set<std::string> gather_resources_from_constraint(const pugi::xml_node &xml_apply) const;
	std::set<std::string> gather_times_from_constraint(const pugi::xml_node &xml_apply) const;

	int constraint_cost(pugi::xml_node xml_node, int points_of_application);

	void parse_prefer_resources_constraint(const pugi::xml_node &xml_node);
	void parse_prefer_times_constraint(const pugi::xml_node &xml_node);
	void parse_avoid_clashes_constraint(const pugi::xml_node &xml_node);
	void parse_split_events_constraint(const pugi::xml_node &xml_node);
	void parse_spread_events_constraint(const pugi::xml_node &xml_node);
	void parse_avoid_unavailable_times_constraint(const pugi::xml_node &xml_node);
	void parse_distribute_split_events_constraint(const pugi::xml_node &xml_node);
	void parse_limit_idle_times_constraint(const pugi::xml_node &xml_node);
	void parse_cluster_busy_times_constraint(const pugi::xml_node &xml_node);



	void parse_constraints();



};

class xmlParserException: public std::exception{
    virtual const char* what() const throw(){
        return "Error: given XML does not follow XHSTT convention. Aborting.";
    }

};

