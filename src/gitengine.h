#ifndef __GITENGINE__
#define __GITENGINE__
extern "C" {
#include "../deps/libgit2/include/git2.h"
#include "../deps/libgit2/include/git2/clone.h"
#include <stdio.h>
#include <string.h>
}

#ifndef PRIuZ
/* Define the printf format specifer to use for size_t output */
#if defined(_MSC_VER) || defined(__MINGW32__)
#	define PRIuZ "Iu"
#else
#	define PRIuZ "zu"
#endif
#endif
#include "util.h"
#include "engine.h"

#ifndef _WIN32
#include <pthread.h>
#include <unistd.h>
#endif

#include <string>
#include <vector>
#include <map>
#include <experimental/filesystem>

class GitEngine {

public:
    GitEngine(void);
    Engine engine;
    int remote_start(const char *url, const char* clone_dir);
    int local_start(const char *repo_location);
};

#endif




