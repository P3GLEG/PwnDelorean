#ifndef __FSENGINE__
#define __FSENGINE__
#include <experimental/filesystem>
#include <fstream>
#include "engine.h"
#include "util.h"

namespace fs = std::experimental::filesystem;

class FilesystemEngine {
    public:
        Engine engine;
        FilesystemEngine(void);
        void start(std::string dir_to_scan);

};
#endif
