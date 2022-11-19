#include "euler_rotation_gen.hpp"

#include <iosfwd>
#include <string>
#include <vector>

#include "../rapid_helpers.hpp"
#include "../rapidxml-1.13/rapidxml.hpp"
#include "../string_helper.hpp"

using namespace std;

EulerRotation parse_euler_rotation(rapidxml::xml_attribute<>* input, vector<XMLError*>*) {
    EulerRotation euler_rotation;
    vector<string> compound_values;
    string attributename;
    euler_rotation.x_rotation = 0;
    euler_rotation.y_rotation = 0;
    euler_rotation.z_rotation = 0;
    attributename = get_attribute_name(input);
    compound_values = split(get_attribute_value(input), ",");
    if (compound_values.size() == 3) {
        euler_rotation.x_rotation = std::stof(compound_values[0]);
        euler_rotation.y_rotation = std::stof(compound_values[1]);
        euler_rotation.z_rotation = std::stof(compound_values[2]);
    }
    return euler_rotation;
}
string stringify_euler_rotation(EulerRotation attribute_value) {
    string output;
    output = to_string(attribute_value.x_rotation);
    output = output + "," + to_string(attribute_value.y_rotation);
    output = output + "," + to_string(attribute_value.z_rotation);
    return output;
}
