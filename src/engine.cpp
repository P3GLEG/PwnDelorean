#include <fstream>
#include "engine.h"
#include "util.h"

RE2::Set filenames_regexes(RE2::DefaultOptions, RE2::UNANCHORED);
RE2::Set content_regexes(RE2::DefaultOptions, RE2::UNANCHORED);
std::vector<std::string> filename_regexes_cache;
std::vector<std::string> content_regexes_cache;
std::map<std::string, piston> filename_matches;
std::map<std::string, piston> content_matches;
std::string *error_holder = NULL;


int Engine::read_patterns_dir(void) {
    fs::path patterns_dir;
    patterns_dir += fs::current_path();
    patterns_dir += "/patterns";
    for (auto &p : fs::directory_iterator(patterns_dir)) {
        LOG_DEBUG << p.path().string(); 
        std::ifstream jsonfile(p.path().string());
        json j;
        jsonfile >> j;
        //TODO: link description of secret found
        //However needed to compile regexes for speed
        for (json::iterator it = j.begin(); it != j.end(); ++it) {
            std::string regex = it.value()["regex"];
            if (it.value()["type"] == "secretFilename") {
                LOG_DEBUG << "Filename Regex : " << regex;
                if(filenames_regexes.Add(regex, error_holder) != FAILURE){
                    filename_regexes_cache.push_back(regex);
                };
            } else {
                LOG_DEBUG << "Content Regex : " << regex;
                if( content_regexes.Add(regex, error_holder) != FAILURE){
                    content_regexes_cache.push_back(regex);
                }
            }
        }
        jsonfile.close();
    }
    
    return SUCCESS;
}

void Engine::Init(void) {
    LOG_DEBUG << "Reving Engine...";
    read_patterns_dir();
    if (!filenames_regexes.Compile() | !content_regexes.Compile()) {
        LOG_ERROR << "Error while compiling regexes";
        exit(FAILURE);
    }

    //TODO:Add check here for if you only want secretfilenames vs content
}

void Engine::output_matches(void){
    json filejarray;
    json contentjarray;
    for (auto i: filename_matches) {
        json tempo;
        piston temp = i.second;
        //LOG_INFO << "Filename match: " << temp.line_matched << " " << temp.oid ;
        tempo[FILENAME] = temp.line_matched;
        tempo[OID] = temp.oid;
        //TODO:Description
        filejarray.push_back(tempo);
    }
    for (auto i: content_matches) {
        piston temp = i.second;
        json tempo;
        tempo[SECRET] = i.first;
        tempo[FILENAME] = temp.path_to_file;
        tempo[LINENUMBER] = temp.linenumber;
        tempo[REGEXES] = temp.regexes_matched;
        tempo[OID] = temp.oid;
        filejarray.push_back(tempo);
        //TODO:Description
        //LOG_INFO<<"[" <<temp.path_to_file << "] Content match: " << i.first << " found by: " << temp.regexes_matched[0] << " " << temp.oid;
    }
    std::cout << filejarray.dump(4) << "\n";
    std::cout << contentjarray.dump(4) << "\n";
}


bool Engine::search_for_content_match(std::string line, int line_number, std::string path, std::string oid) {
    if (content_matches.count(line) > 0) {
        //Already found this no need to do work again
        LOG_DEBUG << "Attempted to parse " << line << " again";
        return true;
    }
    std::vector<int> matched_regexes;
    content_regexes.Match(line, &matched_regexes);
    if (matched_regexes.size() != 0) {
        piston temp;
        for(auto i: matched_regexes){
            temp.regexes_matched.push_back(content_regexes_cache[i]);
        }
        temp.line_matched = line;
        temp.linenumber = line_number;
        temp.path_to_file = path;
        temp.oid = oid;

        content_matches[temp.line_matched] = temp;
        return true;
    }

    return false;
}

bool Engine::search_for_filename_match(std::string filename, std::string oid) {
    if (filename_matches.count(filename) > 0) {
        //Already found this no need to do work again
        LOG_DEBUG << "Attempted to parse " << filename << " again";
        return true;
    }
    std::vector<int> matched_regexes;
    filenames_regexes.Match(filename, &matched_regexes);
    if (matched_regexes.size() != 0) {
        piston temp;
        for (auto i: matched_regexes){
            temp.regexes_matched.push_back(filename_regexes_cache[i]);
        }
        temp.oid = oid;
        temp.line_matched = filename;
        filename_matches[filename] = temp;
        //Fast finish, if you want to add which regex was matched do it here
        return true;
    }
    return false;
}

