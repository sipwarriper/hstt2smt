#include "XHSTTPrinterModel.h"
#include <iostream>
#include "XMLParser.h"


XHSTTPrinterModel::XHSTTPrinterModel(std::string filename): Model(){
	auto parser = XMLParser(this, filename);
	parser.parse_model();
}

void XHSTTPrinterModel::register_time(const std::string& id, const std::string& name){
	Model::register_time(id, name);
	std::cout << "Register time: id = " << id << " name = " << name << std::endl;
}

void XHSTTPrinterModel::register_event(const std::string& id, const std::string& name, const int& duration,
	const std::string& color){
	Model::register_event(id, name, duration, color);
	std::cout << "Register event: id = " << id << " name = " << name << std::endl;
}

void XHSTTPrinterModel::register_resource(std::string id, std::string name, std::string rtype){
	Model::register_resource(id, name, rtype);
	std::cout << "Register resource: id = " << id << " name = " << name << std::endl;
}

void XHSTTPrinterModel::register_resource_type(std::string id, std::string name){
	Model::register_resource_type(id, name);
	std::cout << "Register resource type: id = " << id << " name = " << name << std::endl;
}

void XHSTTPrinterModel::set_time_for_event(int event_num, int duration, Time* start_t){
	std::cout << "set_time_for_event" << std::endl;
}

void XHSTTPrinterModel::prefer_resources_constraint(const std::set<std::string>& events_ids,
	const std::set<std::string>& resources_ids, const std::string& role){
	std::cout << "prefer_resources_constraint" << std::endl;
}

void XHSTTPrinterModel::prefer_times_constraint(const int& cost, const std::set<std::string>& events_ids,
	const std::set<std::string>& times_ids, int duration){
	std::cout << "prefer_times_constraint" << std::endl;
}

void XHSTTPrinterModel::avoid_clashes_constraint(const int& cost, const std::set<std::string>& resources_ids){
	std::cout << "avoid_clashes_constraint" << std::endl;
}

void XHSTTPrinterModel::split_events_constraint(const int& cost, const std::set<std::string>& events, const int& min,
	const int& max, const int& min_amount, const int& max_amount){
	std::cout << "split_events_constraint" << std::endl;
}

void XHSTTPrinterModel::spread_events_constraint(const int& cost, const std::set<std::string>& event_groups,
	std::unordered_map<std::string, std::pair<int, int>> time_groups){
	std::cout << "spread_events_constraint" << std::endl;
}

void XHSTTPrinterModel::avoid_unavailable_times_constraint(const int& cost, const std::set<std::string>& resources_ids,
	const std::set<std::string>& times_ids){
	std::cout << "spread_events_constraint" << std::endl;
}

void XHSTTPrinterModel::distribute_split_events_constraints(const int& cost, const std::set<std::string>& event_ids,
	const int& duration, const int& min, const int& max){
	std::cout << "distribute_split_events_constraints" << std::endl;
}

void XHSTTPrinterModel::limit_idle_times_constraint(const int& cost, const std::set<std::string>& resources_ids,
	const std::set<std::string>& time_groups_ids, const int& min, const int& max){
	std::cout << "limit_idle_times_constraint" << std::endl;
}

void XHSTTPrinterModel::cluster_busy_times_constraint(const int& cost, const std::set<std::string>& resources_ids,
	const std::set<std::string>& time_groups_ids, const int& min, const int& max){
	std::cout << "cluster_busy_times_constraint" << std::endl;
}

void XHSTTPrinterModel::on_parsed_events(bool spread_constraint){
	std::cout << "on_parsed_events" << std::endl;
}







