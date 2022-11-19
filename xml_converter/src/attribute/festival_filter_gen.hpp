#pragma once

#include <string>
#include <vector>

#include "../rapidxml-1.13/rapidxml.hpp"

class XMLError;

class FestivalFilter {
 public:
    bool dragonbash;
    bool festival_of_the_four_winds;
    bool halloween;
    bool lunar_new_year;
    bool none;
    bool super_adventure_festival;
    bool wintersday;

    virtual std::string classname() {
        return "FestivalFilter";
    };
};
FestivalFilter parse_festival_filter(rapidxml::xml_attribute<>* input, std::vector<XMLError*>* errors);
std::string stringify_festival_filter(FestivalFilter attribute_value);
