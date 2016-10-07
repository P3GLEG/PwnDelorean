
#include <experimental/filesystem>
#include <fstream>
#include "engine.h"
#include "util.h"

namespace fs = std::experimental::filesystem;

class FilesystemEngine {
    public:
        FilesystemEngine(void);
        void start(std::string dir_to_scan);

};