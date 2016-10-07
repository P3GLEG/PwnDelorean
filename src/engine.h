#include <re2.h>
#include <re2/set.h>
#include <string>
#include <experimental/filesystem>
#include "../deps/json/src/json.hpp"
#include <plog/Log.h>
#include <plog/Appenders/ColorConsoleAppender.h>


using json = nlohmann::json;
namespace fs = std::experimental::filesystem;

struct piston{
    std::vector<std::string> regexes_matched;
    std::string regex_matched;
    std::string path_to_file;
    std::string line_matched;
    std::string oid;
    int linenumber;
};

class Engine {
public:
    bool search_for_content_match(std::string line, int linenumber, std::string path, std::string oid);

    bool search_for_filename_match(std::string filename, std::string oid);

    std::map<std::string, piston> filename_matches;
    std::map<std::string, piston> content_matches;

    void Init(void);
    void Shutdown();

private:
    int read_patterns_dir(void);

};
