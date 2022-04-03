#include "string.hpp"

#include <string>
#include <vector>

#include "../rapidxml-1.13/rapidxml.hpp"

using namespace std;

string parse_string(rapidxml::xml_attribute<>* input, vector<string> *errors) {
    return string(input->value());
}
