#ifndef XMLWRITTER_H
#define XMLWRITTER_H

#include<iostream>
#include<pugixml.hpp>
#include <memory>

#include "Model.h"

class XMLWritter
{
public:
    XMLWritter();
    XMLWritter(std::string filename, std::shared_ptr<Model> instance, std::string id = "TFG_IsmaelElHabri_2019");
private:
    void inflate_events_tags(pugi::xml_node events, std::shared_ptr<Model> instance);
};

#endif // XMLWRITTER_H
