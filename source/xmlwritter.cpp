#include "xmlwritter.h"
#include <filesystem>
#include <memory>
#include <ctime>
#include <string>

#include "Model.h"

XMLWritter::XMLWritter() {}

XMLWritter::XMLWritter(std::string filename, std::shared_ptr<Model> instance, std::string id)
{
    pugi::xml_document doc;

    pugi::xml_parse_result result = doc.load_file(filename.c_str());

    if (!std::filesystem::is_directory("output")) { // Check if output folder exists
        std::filesystem::create_directory("output"); // create output folder
    }

    pugi::xml_node xml_node= doc.root().child("HighSchoolTimetableArchive").child("SolutionGroups");
    if(!xml_node){
        xml_node = doc.root().child("HighSchoolTimetableArchive").append_child("SolutionGroups");
    }

    pugi::xml_node xml_solution_group = xml_node.append_child("SolutionGroup");
    xml_solution_group.append_attribute("Id") = id.c_str();

    pugi::xml_node xml_metadata = xml_solution_group.append_child("MetaData");
    pugi::xml_node node = xml_metadata.append_child("Contributor");
	node.append_child(pugi::node_pcdata).set_value("Ismael El Habri");
    node = xml_metadata.append_child("Date");

	time_t now = time(0);
	tm *ltm = localtime(&now);
	std::string date = std::to_string(ltm->tm_mday) + std::to_string(1+ltm->tm_mon) + std::to_string(1900+ltm->tm_year);

	node.append_child(pugi::node_pcdata).set_value(date.c_str());

    node = xml_metadata.append_child("Description");
    node.set_value("Solution obtained with a SMT modelling powered by yices");

    pugi::xml_node solution = xml_solution_group.append_child("Solution");
    solution.append_attribute("Reference") = doc.root().child("HighSchoolTimetableArchive").child("Instances").child("Instance")
            .attribute("Id").as_string();
    pugi::xml_node events = solution.append_child("Events");

	inflate_events_tags(events, instance);

    //write element tree to a file

    std::string fn = std::filesystem::path(filename).filename();
    fn = "output/"+fn;

    doc.save_file(fn.c_str());

}


void XMLWritter::inflate_events_tags(pugi::xml_node events, std::shared_ptr<Model> instance){

    std::vector<AssignmentEntry> time_assignments = instance->get_time_assignments();

    for(AssignmentEntry entry : time_assignments){
        pugi::xml_node event_tag = events.append_child("Event");
        event_tag.append_attribute("Reference") = entry.event_id.c_str();
        pugi::xml_node tag = event_tag.append_child("Duration");
        tag.append_child(pugi::node_pcdata).set_value(std::to_string(entry.duration).c_str());
        tag = event_tag.append_child("Time");
        tag.append_attribute("Reference") = entry.time_id.c_str();
    }


}
