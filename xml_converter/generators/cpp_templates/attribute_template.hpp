#pragma once

#include <string>
#include <vector>

#include "../rapidxml-1.13/rapidxml.hpp"

class XMLError;

{% if type == "Enum":%}
enum {{class_name}} {
    {% for attribute_variable in attribute_variables: %}
    {{attribute_variable.attribute_name}},
    {% endfor %}
};
{% else: %}
class {{class_name}} {
 public:
    {% for attribute_variable in attribute_variables: %}
    {{attribute_variable.cpp_type}} {{attribute_variable.attribute_name}};
    {% endfor %}

    virtual std::string classname() {
        return "{{class_name}}";
    }
};
{% endif %}
{{class_name}} parse_{{attribute_name}}(rapidxml::xml_attribute<>* input, std::vector<XMLError*>* errors);
std::string stringify_{{attribute_name}}({{class_name}} attribute_value);
