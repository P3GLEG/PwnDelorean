extern "C" {
#include "../deps/libgit2/include/git2.h"
#include "../deps/libgit2/include/git2/clone.h"
#include <stdio.h>
#include <string.h>
}

#include "util.h"

#ifndef _WIN32
#include <pthread.h>
#include <unistd.h>
#endif

#include <string>
#include <vector>

class GitEngine {

public:
    GitEngine(void);
    int remote_start(const char *url, const char* clone_dir);
    int local_start(const char *repo_location);
};





