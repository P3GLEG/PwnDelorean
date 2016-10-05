/*
 * @Author Paul Ganea
 */
extern "C" {
#include "../deps/libgit2/include/git2.h"
}
#ifndef _WIN32

#endif

#include <map>
#include <fstream>
#include <re2.h>
#include <re2/set.h>
#include "gitengine.h"


int init(void) {
    static plog::RollingFileAppender<plog::CsvFormatter> fileAppender("debug.txt", 100000000, 3);
    static plog::ColorConsoleAppender<plog::TxtFormatter> consoleAppender;
    plog::init(plog::verbose, &consoleAppender).addAppender(&fileAppender);
    return SUCCESS;
}

int main(int argc, char *argv[]) {
    if (init() != 0) {
        LOG_ERROR << "Failed to initialize the program";
        exit(FAIL);
    }
    std::string url("https://github.com/pegleg2060/PwnDelorean.git");
    GitEngine git;
    git.start(url);
    system("rm -r /tmp/pwndelorean");
    return SUCCESS;
}

