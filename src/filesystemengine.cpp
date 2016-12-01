//
// Created by root on 10/7/16.
//
#include "filesystemengine.h"
#include "util.h"

Engine* e1;

FilesystemEngine::FilesystemEngine(void) {
    this->engine.Init();
    e1 = &this->engine;
}

std::vector<std::string> binary_files = {".dll", ".so"}; //These file types typically always match and give junk data

bool FilesystemEngine::isBinary(std::string file_ext){
//    return std::find(binary.begin(), binary.end(), filename) != vector.end();
return true;
}

void FilesystemEngine::start(std::string dir_to_scan) {
    for (auto &path: fs::recursive_directory_iterator(dir_to_scan)) {
        LOG_DEBUG << "Scanning in Directory: " << path.path().generic_string();
            if(!fs::is_directory(path.path())) {
                LOG_DEBUG << "filename found :" << path.path().generic_string();
                /*
                if(isBinary(path.path().generic_string().extension){
                    LOG_DEBUG << "It's binary ignoring...:" << path.path().generic_string();
                    continue;
                }
                 */
                e1->search_for_filename_match(path.path(), "", "");
                std::ifstream file(path.path());
                int line_number = 1;
                if (file) {

                    for (std::string line; getline(file, line);) {
                        e1->search_for_content_match(line, line_number, path.path(), "");
                        line_number++;
                    }

                } else {
                    LOG_ERROR << "Unable to open file: " << path.path().generic_string();
                }
            }
        }
    e1->output_matches();
}

