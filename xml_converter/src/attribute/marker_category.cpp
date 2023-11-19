#include "marker_category.hpp"

#include <string>
#include <vector>

#include "../rapid_helpers.hpp"
#include "../rapidxml-1.13/rapidxml.hpp"
#include "waypoint.pb.h"

////////////////////////////////////////////////////////////////////////////////
// parse_marker_category
//
// Parses a MarkerCategory from the value of a rapidxml::xml_attribute.
////////////////////////////////////////////////////////////////////////////////
void xml_attribute_to_marker_category(
    rapidxml::xml_attribute<>* input,
    std::vector<XMLError*>* errors,
    XMLReaderState* state,
    MarkerCategory* value,
    bool* is_set) {
    value->category = get_attribute_value(input);
    *is_set = true;
}

////////////////////////////////////////////////////////////////////////////////
// marker_category_to_xml_attribute
//
// Converts a marker category a fully qualified xml attribute string.
////////////////////////////////////////////////////////////////////////////////
std::string marker_category_to_xml_attribute(const std::string& attribute_name, const MarkerCategory* value) {
    return " " + attribute_name + "=\"" + value->category + "\"";
}

////////////////////////////////////////////////////////////////////////////////
// proto_to_marker_category
//
// Parses a marker category from a proto field.
////////////////////////////////////////////////////////////////////////////////
void proto_to_marker_category(waypoint::Category input, MarkerCategory* value, bool* is_set) {
    MarkerCategory marker_category;
    marker_category.category = input.name();
    *value = marker_category;
    *is_set = true;
}

////////////////////////////////////////////////////////////////////////////////
// to_proto_marker_category
//
// Returns a waypoint::Category so that it can be saved to proto.
////////////////////////////////////////////////////////////////////////////////
void marker_category_to_proto(MarkerCategory value, std::function<void(waypoint::Category*)> setter) {
    waypoint::Category* category = new waypoint::Category();
    category->set_name(value.category);
    setter(category);
}
