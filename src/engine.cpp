#include <fstream>
#include "engine.h"
#include "util.h"


RE2::Set filenames_regexes(RE2::DefaultOptions, RE2::UNANCHORED);
RE2::Set content_regexes(RE2::DefaultOptions, RE2::UNANCHORED);
std::map<std::string, bool> filename_matches;
std::map<std::string, bool> content_matches;
std::string *error_holder = NULL;


int Engine::read_patterns_dir(void) {
    fs::path patterns_dir;
    patterns_dir += fs::current_path();
    patterns_dir += "/patterns";
    for (auto &p : fs::directory_iterator(patterns_dir)) {
        std::ifstream jsonfile(p.path().string());
        json j;
        jsonfile >> j;
        //TODO: link description of secret found
        //However needed to compile regexes for speed
        for (json::iterator it = j.begin(); it != j.end(); ++it) {
            std::string regex = it.value()["regex"];
            if (it.value()["type"] == "secretFilename") {
                LOG_DEBUG << "Filename Regex : " << regex;
                filenames_regexes.Add(regex, error_holder);
            } else {
                LOG_DEBUG << "Content Regex : " << regex;
                content_regexes.Add(regex, error_holder);
            }
        }
        jsonfile.close();
    }
    if (!filenames_regexes.Compile() | !content_regexes.Compile()) {
        LOG_ERROR << "Error while compiling regexes";
        return FAIL;
    }
    return SUCCESS;
}

void Engine::Init(void) {
    read_patterns_dir();
}


bool Engine::search_for_content_match(std::string line) {
    if (content_matches.count(line) > 0) {
        //Already found this no need to do work again
        LOG_DEBUG << "Attempted to parse " << line << " again";

        return true;
    }
    std::vector<int> matched_regexes;
    content_regexes.Match(line, &matched_regexes);
    if (matched_regexes.size() != 0) {
        for (auto i: matched_regexes) {
            LOG_ERROR << i;
        }
        content_matches[line] = true;
        //Fast finsh, if you want to add which regex was matched do it here
        return true;
    }

    return false;
}

bool Engine::search_for_filename_match(std::string filename) {
    if (filename_matches.count(filename) > 0) {
        //Already found this no need to do work again
        LOG_DEBUG << "Attempted to parse " << filename << " again";
        return true;
    }
    std::vector<int> matched_regexes;
    filenames_regexes.Match(filename, &matched_regexes);
    if (matched_regexes.size() != 0) {
        filename_matches[filename] = true;
        //Fast finish, if you want to add which regex was matched do it here
        return true;
    }
    return false;
}
