#include "libgit2/include/git2.h"
#include "libgit2/include/git2/clone.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef _WIN32
# include <pthread.h>
# include <unistd.h>
#endif


int main(void){
    int error = 0;
    git_libgit2_init();
    git_repository *repo = NULL;
    error = git_clone(&repo,"https://github.com/libgit2/libgit2" , "/tmp/yay", NULL);
    if (error != 0) {
        printf("ERROR: %s\n", giterr_last()->message);
    }
    return 0;
        
}
