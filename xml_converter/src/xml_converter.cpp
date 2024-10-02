#include <dirent.h>
#include <string.h>
#include <sys/stat.h>

#include <algorithm>
#include <chrono>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <map>
#include <set>
#include <string>
#include <system_error>
#include <utility>
#include <vector>

#include "attribute/marker_category.hpp"
#include "category_gen.hpp"
#include "file_helper.hpp"
#include "guildpoint.pb.h"
#include "icon_gen.hpp"
#include "packaging_protobin.hpp"
#include "packaging_xml.hpp"
#include "parseable.hpp"
#include "rapid_helpers.hpp"
#include "rapidxml-1.13/rapidxml.hpp"
#include "rapidxml-1.13/rapidxml_utils.hpp"
#include "string_helper.hpp"
#include "trail_gen.hpp"

using namespace std;

bool filename_comp(string a, string b) {
    return lowercase(a) < lowercase(b);
}

vector<string> get_files_by_suffix(string directory, string suffix) {
    vector<string> files;
    DIR* dir = opendir(directory.c_str());
    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        string filename = entry->d_name;
        if (filename != "." && filename != "..") {
            string path = join_file_paths(directory, filename);
            if (entry->d_type == DT_DIR) {
                vector<string> subfiles = get_files_by_suffix(path, suffix);
                // Default: markerpacks have all xml files in the first directory
                for (string subfile : subfiles) {
                    cout << subfile << " found in subfolder" << endl;
                    files.push_back(subfile);
                }
            }
            else if (has_suffix(filename, suffix)) {
                files.push_back(path);
            }
        }
    }
    closedir(dir);
    std::sort(files.begin(), files.end(), filename_comp);
    return files;
}

set<string> read_taco_directory(
    string input_path,
    map<string, Category>* marker_categories,
    vector<Parseable*>* parsed_pois) {
    set<string> top_level_categories;
    if (!filesystem::exists(input_path)) {
        cout << "Error: " << input_path << " is not an existing directory or file" << endl;
    }
    else if (filesystem::is_directory(input_path)) {
        vector<string> xml_files = get_files_by_suffix(input_path, ".xml");
        for (const string& path : xml_files) {
            set<string> category_names = (parse_xml_file(path, input_path, marker_categories, parsed_pois));
            top_level_categories.insert(category_names.begin(), category_names.end());
        }
    }
    else if (filesystem::is_regular_file(input_path)) {
        set<string> category_names = parse_xml_file(input_path, get_base_dir(input_path), marker_categories, parsed_pois);
        top_level_categories.insert(category_names.begin(), category_names.end());
    }
    return top_level_categories;
}

set<string> read_burrito_directory(
    string input_path,
    map<string, Category>* marker_categories,
    vector<Parseable*>* parsed_pois) {
    set<string> top_level_categories;
    if (!filesystem::exists(input_path)) {
        cout << "Error: " << input_path << " is not an existing directory or file" << endl;
    }
    else if (filesystem::is_directory(input_path)) {
        vector<string> burrito_files = get_files_by_suffix(input_path, ".bin");
        for (const string& path : burrito_files) {
            set<string> category_names = read_protobuf_file(path, input_path, marker_categories, parsed_pois);
            top_level_categories.insert(category_names.begin(), category_names.end());
        }
    }
    else if (filesystem::is_regular_file(input_path)) {
        set<string> category_names = read_protobuf_file(input_path, get_base_dir(input_path), marker_categories, parsed_pois);
        top_level_categories.insert(category_names.begin(), category_names.end());
    }
    return top_level_categories;
}

void write_taco_directory(
    string output_path,
    map<string, Category>* marker_categories,
    vector<Parseable*>* parsed_pois) {
    // TODO: Exportion of XML Marker Packs File Structure #111
    if (!filesystem::is_directory(output_path)) {
        if (!filesystem::create_directory(output_path)) {
            cout << "Error: " << output_path << "is not a valid directory path" << endl;
            return;
        }
    }
    write_xml_file(output_path, marker_categories, parsed_pois);
}

void write_burrito_directory(
    string output_path,
    map<string, Category>* marker_categories,
    vector<Parseable*>* parsed_pois) {
    if (!filesystem::is_directory(output_path)) {
        if (!filesystem::create_directory(output_path)) {
            cout << "Error: " << output_path << "is not a valid directory path" << endl;
            return;
        }
    }
    StringHierarchy category_filter;
    category_filter.add_path({}, true);
    write_protobuf_file(output_path, category_filter, marker_categories, parsed_pois);
}

////////////////////////////////////////////////////////////////////////////////
// process_data
//
// The universal entrypoint into the xml converter functionality. Both the CLI
// and the library entrypoints direct here to do their actual processing.
////////////////////////////////////////////////////////////////////////////////
void process_data(
    vector<string> input_taco_paths,
    vector<string> input_guildpoint_paths,
    // If multiple inputs are found to have the same top level categories,
    // The program will skip writing to output unless the below is true
    bool allow_duplicates,
    // These will eventually have additional arguments for each output path to
    // allow for splitting out a single markerpack
    vector<string> output_taco_paths,
    vector<string> output_guildpoint_paths,

    // This is a special output path used for burrito internal use that splits
    // the guildpoint protobins by map id.
    string output_split_guildpoint_dir) {
    // All of the loaded pois and categories
    vector<Parseable*> parsed_pois;
    map<string, Category> marker_categories;
    set<string> top_level_categories;
    vector<string> duplicate_categories;

    // Read in all the xml taco markerpacks
    auto begin = chrono::high_resolution_clock::now();
    for (size_t i = 0; i < input_taco_paths.size(); i++) {
        cout << "Loading taco pack " << input_taco_paths[i] << endl;

        set<string> category_names = read_taco_directory(
            input_taco_paths[i],
            &marker_categories,
            &parsed_pois);
        combine_sets(&category_names, &top_level_categories, &duplicate_categories);
    }
    auto end = chrono::high_resolution_clock::now();
    auto dur = end - begin;
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(dur).count();
    cout << "The taco parse function took " << ms << " milliseconds to run" << endl;

    // Read in all the protobin guildpoint markerpacks
    for (size_t i = 0; i < input_guildpoint_paths.size(); i++) {
        cout << "Loading guildpoint pack " << input_guildpoint_paths[i] << endl;

        set<string> category_names = read_burrito_directory(
            input_guildpoint_paths[i],
            &marker_categories,
            &parsed_pois);
        combine_sets(&category_names, &top_level_categories, &duplicate_categories);
    }

    if (duplicate_categories.size() > 0 && allow_duplicates != true) {
        cout << "Did not write due to duplicates in categories. If you want to bypass this, use '--allow-duplicates'" << endl;
        cout << "The following top level categories were found in more than one pack" << endl;
        for (size_t i = 0; i < duplicate_categories.size(); i++) {
            cout << duplicate_categories[i] << endl;
        }
        return;
    }

    // Write all of the xml taco paths
    begin = chrono::high_resolution_clock::now();
    for (size_t i = 0; i < output_taco_paths.size(); i++) {
        write_taco_directory(output_taco_paths[i], &marker_categories, &parsed_pois);
    }
    end = chrono::high_resolution_clock::now();
    dur = end - begin;
    ms = std::chrono::duration_cast<std::chrono::milliseconds>(dur).count();
    cout << "The xml write function took " << ms << " milliseconds to run" << endl;

    // Write all of the protobin guildpoint paths
    for (size_t i = 0; i < output_guildpoint_paths.size(); i++) {
        write_burrito_directory(output_guildpoint_paths[i], &marker_categories, &parsed_pois);
    }

    // Write the special map-split protbin guildpoint file
    begin = chrono::high_resolution_clock::now();
    if (output_split_guildpoint_dir != "") {
        StringHierarchy category_filter;
        category_filter.add_path({}, true);
        write_protobuf_file_per_map_id(output_split_guildpoint_dir, category_filter, &marker_categories, &parsed_pois);
    }
    end = chrono::high_resolution_clock::now();
    dur = end - begin;
    ms = std::chrono::duration_cast<std::chrono::milliseconds>(dur).count();
    cout << "The protobuf write function took " << ms << " milliseconds to run" << endl;
}

////////////////////////////////////////////////////////////////////////////////
// main
//
// Main is the CLI entrypoint to the xml converter. It handles processing all
// of the command line data into a format the internal functions want to
// receive.
//
// Example usage
//   ./xml_converter --input-taco-paths ../packs/marker_pack --output-split-guildpoint-path ../output_packs
//   ./xml_converter --input-taco-paths ../packs/* --output-split-guildpoint-path ../output_packs
////////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[]) {
    vector<string> input_taco_paths;
    vector<string> output_taco_paths;
    vector<string> input_guildpoint_paths;
    vector<string> output_guildpoint_paths;
    bool allow_duplicates = false;

    // Typically "~/.local/share/godot/app_userdata/Burrito/protobins" for
    // converting from xml markerpacks to internal protobuf files.
    vector<string> output_split_guildpoint_paths;

    vector<string>* arg_target = &input_taco_paths;

    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "--input-taco-path")) {
            arg_target = &input_taco_paths;
        }
        else if (!strcmp(argv[i], "--output-taco-path")) {
            arg_target = &output_taco_paths;
        }
        else if (!strcmp(argv[i], "--input-guildpoint-path")) {
            arg_target = &input_guildpoint_paths;
        }
        else if (!strcmp(argv[i], "--output-guildpoint-path")) {
            arg_target = &output_guildpoint_paths;
        }
        else if (!strcmp(argv[i], "--output-split-guildpoint-path")) {
            // We dont actually support multiple values for this argument but
            // I am leaving this as-is because it is simpler. We can adjust the
            // CLI arg parsing later to properly capture this.
            arg_target = &output_split_guildpoint_paths;
        }
        else if (!strcmp(argv[i], "--allow-duplicates")) {
            allow_duplicates = true;
            arg_target = nullptr;
        }
        else {
            if (arg_target != nullptr) {
                arg_target->push_back(argv[i]);
            }
            else {
                cout << "Unknown argument " << argv[i] << endl;
                return -1;
            }
        }
    }

    // Strip all but the first output split guildpoint argument, because we dont
    // actually support multiple arguments.
    string output_split_guildpoint_dir = "";
    if (output_split_guildpoint_paths.size() > 0) {
        output_split_guildpoint_dir = output_split_guildpoint_paths[0];
    }
    else if (output_split_guildpoint_paths.size() > 1) {
        cout << "Only one --output-split-guildpoint-path is accepted" << endl;
    }

    process_data(
        input_taco_paths,
        input_guildpoint_paths,
        allow_duplicates,
        output_taco_paths,
        output_guildpoint_paths,
        output_split_guildpoint_dir);

    return 0;
}
