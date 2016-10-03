extern "C" {
    #include "../libgit2/include/git2.h"
    #include "../libgit2/include/git2/clone.h"
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>

}
#ifndef _WIN32
    # include <pthread.h>
    # include <unistd.h>
#endif
#include <iostream>
#include <queue>
#include <unordered_map>
#include <string>

#define OUTPUT_DIR "/tmp/yay/"
#define REPO OUTPUT_DIR ".git"

using namespace std;



void printGitError(){
    printf("ERROR: %s\n", giterr_last()->message);
}

static void print_signature(const char *header, const git_signature *sig)
{
    char sign;
    int offset, hours, minutes;

    if (!sig)
        return;

    offset = sig->when.offset;
    if (offset < 0) {
        sign = '-';
        offset = -offset;
    } else {
        sign = '+';
    }

    hours   = offset / 60;
    minutes = offset % 60;

    printf("%s %s <%s> %ld %c%02d%02d\n",
           header, sig->name, sig->email, (long)sig->when.time,
           sign, hours, minutes);
}
static void show_commit(const git_commit *commit)
{
    unsigned int i, max_i;
    char oidstr[GIT_OID_HEXSZ + 1];

    git_oid_tostr(oidstr, sizeof(oidstr), git_commit_tree_id(commit));
    printf("tree %s\n", oidstr);

    max_i = (unsigned int)git_commit_parentcount(commit);
    for (i = 0; i < max_i; ++i) {
        git_oid_tostr(oidstr, sizeof(oidstr), git_commit_parent_id(commit, i));
        printf("parent %s\n", oidstr);
    }

    print_signature("author", git_commit_author(commit));
    print_signature("committer", git_commit_committer(commit));

    if (git_commit_message(commit))
        printf("\n%s\n", git_commit_message(commit));
}


int test(git_repository *repo){

    char head_filepath[512];
    FILE *head_fileptr;
    char head_rev[41];

    strcpy(head_filepath, REPO);

    if(strrchr(REPO, '/') != (REPO+strlen(REPO)))
        strcat(head_filepath, "/refs/heads/master");
    else
        strcat(head_filepath, "refs/heads/master");


    if((head_fileptr = fopen(head_filepath, "r")) == NULL){
        fprintf(stderr, "Error opening '%s'\n", head_filepath);
        return 1;
    }

    if(fread(head_rev, 40, 1, head_fileptr) != 1){
        fprintf(stderr, "Error reading from '%s'\n", head_filepath);
        fclose(head_fileptr);
        return 1;
    }

    fclose(head_fileptr);
    git_oid oid;
    git_revwalk *walker;
    git_commit *commit;

    if(git_oid_fromstr(&oid, head_rev) != 0){
        fprintf(stderr, "Invalid git object: '%s'\n", head_rev);
        return 1;
    }

    git_revwalk_new(&walker, repo);
    git_revwalk_sorting(walker, GIT_SORT_TOPOLOGICAL);
    git_revwalk_push(walker, &oid);

    const char *commit_message;
    const git_signature *commit_author;

    while(git_revwalk_next(&oid, walker) == 0) {
        if(git_commit_lookup(&commit, repo, &oid)){
            fprintf(stderr, "Failed to lookup the next object\n");
            return 1;
        }
        show_commit(commit);
        git_commit_free(commit);
    }

    git_revwalk_free(walker);
}
int main(int argc, char *argv[]){
    int error = 0;
    git_libgit2_init();
    git_repository *repo = NULL;
    error = git_clone(&repo,"https://github.com/pegleg2060/PwnDelorean.git" , OUTPUT_DIR, NULL);
    if (error != 0) {
        printGitError();
        exit(1);
    }
    printf("Git repo saved at: %s\n", git_repository_path(repo));
    test(repo);
    git_repository_free(repo);
    system("rm -r /tmp/yay");
    return 0;
        
}


