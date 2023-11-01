#include "{{attribute_name}}_gen.hpp"

#include <algorithm>
#include <iosfwd>
#include <string>
#include <vector>

#include "../rapid_helpers.hpp"
#include "../rapidxml-1.13/rapidxml.hpp"
#include "../string_helper.hpp"
#include "waypoint.pb.h"

using namespace std;

void xml_attribute_to_{{attribute_name}}(
    rapidxml::xml_attribute<>* input,
    std::vector<XMLError*>* errors,
    {{class_name}}* value,
    bool* is_set) {
    {{class_name}} {{attribute_name}};
    string normalized_value = normalize(get_attribute_value(input));
    {% for n, attribute_variable in enumerate(attribute_variables) %}
        {% for i, value in enumerate(attribute_variable.xml_fields) %}
            {% if i == 0 and n == 0: %}
                if (normalized_value == "{{value}}") {
                    {{attribute_name}} = {{class_name}}::{{attribute_variable.attribute_name}};
                }
            {% else %}
                else if (normalized_value == "{{value}}") {
                    {{attribute_name}} = {{class_name}}::{{attribute_variable.attribute_name}};
                }
            {% endif %}
        {% endfor %}
    {% endfor %}
    else {
        errors->push_back(new XMLAttributeValueError("Found an invalid value that was not in the Enum {{class_name}}", input));
        {{attribute_name}} = {{class_name}}::{{attribute_variables[0].attribute_name}};
    }
    *value = {{attribute_name}};
    *is_set = true;
}

string {{attribute_name}}_to_xml_attribute(const std::string& attribute_name, const {{class_name}}* value) {
    {% for n, attribute_variable in enumerate(attribute_variables) %}
        {% for i, value in enumerate(attribute_variable.xml_fields) %}
            {%-if i == 0 and n == 0:%}
                if (*value == {{class_name}}::{{attribute_variable.attribute_name}}) {
            {% else %}
                else if (*value == {{class_name}}::{{attribute_variable.attribute_name}}) {
            {%  endif %}
                return " " + attribute_name + "=\"" + "{{value}}" + "\"";
            }
        {%  endfor %}
    {%  endfor %}
    else {
        return " " + attribute_name + "=\"" + "{{class_name}}::{{attribute_variables[0].xml_fields[0]}}" + "\"";
    }
}

{{proto_field_cpp_type}} to_proto_{{attribute_name}}({{class_name}} attribute_value) {
    switch (attribute_value) {
        {% for attribute_variable in attribute_variables %}
            case {{class_name}}::{{attribute_variable.attribute_name}}:
                return {{proto_field_cpp_type}}::{{attribute_variable.attribute_name}};
        {% endfor %}
        default:
            return {{proto_field_cpp_type}}::{{attribute_variables[0].attribute_name}};
    }
}

{{class_name}} from_proto_{{attribute_name}}({{proto_field_cpp_type}} proto_{{attribute_name}}) {
    switch (proto_{{attribute_name}}) {
        {% for attribute_variable in attribute_variables %}
            case {{proto_field_cpp_type}}::{{attribute_variable.attribute_name}}:
                return {{class_name}}::{{attribute_variable.attribute_name}};
        {% endfor %}
        default:
            return {{class_name}}::{{attribute_variables[0].attribute_name}};
    }
}
