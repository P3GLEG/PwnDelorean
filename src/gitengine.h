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


#define OUTPUT_DIR "/tmp/pwndelorean/"
#define REPO OUTPUT_DIR ".git"

class GitEngine {

public:
    int start(const char *url, const char* output_dir);

};





