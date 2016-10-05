#include <re2.h>
#include <re2/set.h>
#include <string>
#include <experimental/filesystem>
#include "../deps/json/src/json.hpp"
#include <plog/Log.h>
#include <plog/Appenders/ColorConsoleAppender.h>


using json = nlohmann::json;
namespace fs = std::experimental::filesystem;

class Engine {
public:
    bool search_for_content_match(std::string line);

    bool search_for_filename_match(std::string filename);

    std::map<std::string, bool> filename_matches;
    std::map<std::string, bool> content_matches;

    void Init(void);

private:
    int read_patterns_dir(void);

};
