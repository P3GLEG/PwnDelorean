extern "C" {
    #include "../deps/libgit2/include/git2.h"
    #include "../deps/libgit2/include/git2/clone.h"
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
}
#ifndef _WIN32
    # include <pthread.h>
    # include <unistd.h>
#endif
#include <experimental/filesystem>
#include <iostream>
#include <dirent.h>
#include <unordered_map>
#include <fstream>
#include "../deps/json/src/json.hpp"
#include <plog/Log.h>
#include <plog/Appenders/ColorConsoleAppender.h>


#define OUTPUT_DIR "/tmp/yay/"
#define REPO OUTPUT_DIR ".git"

using json = nlohmann::json;
using namespace std;
namespace fs = std::experimental::filesystem;
static vector<json> filenames_to_find;
static vector<json> content_to_find;
git_repository *repo = NULL;
unordered_map <string, bool> blobs;

void parse_blob(const git_blob *blob)
{
        fwrite(git_blob_rawcontent(blob), (size_t)git_blob_rawsize(blob), 1, stdout);
}

void print_git_error(){
    LOG_ERROR << giterr_last()->message;
}


void parse_tree_entry(const git_tree_entry *entry){
    git_otype type = git_tree_entry_type(entry);
    switch(type){
        case GIT_OBJ_BLOB:
            git_object* blob;
            char oidstr[GIT_OID_HEXSZ + 1];
            git_oid_tostr(oidstr, sizeof(oidstr), git_tree_entry_id(entry));
            git_tree_entry_to_object(&blob, repo, entry);
            if(blobs[oidstr]){
                cout << "Already parsed" <<endl;
                //
            }else{
                parse_blob((const git_blob *)blob);
                blobs[oidstr] = true;
            }
            break;
        default:
            break;
    }


}
int tree_walk_cb(const char *root, const git_tree_entry *entry, void *payload)
{
    parse_tree_entry(entry);
    return 0;
}

int parse_commit_tree(git_repository *repo, const git_commit *commit){
    int error = 0;
    git_tree *tree = NULL;
    error = git_tree_lookup(&tree, repo, git_commit_tree_id(commit));
    if(error != 0){
        print_git_error();
        return -1;
    }
    error = git_tree_walk(tree, GIT_TREEWALK_PRE, tree_walk_cb, NULL);
        if(error != 0){
        print_git_error();
        return -1;
    }
    return 0;
}


int start(git_repository *repo){
    char head_filepath[512];
    FILE *head_fileptr;
    char head_rev[41];
    strcpy(head_filepath, REPO);
    //TODO: Add different refs
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
    while(git_revwalk_next(&oid, walker) == 0) {
        if(git_commit_lookup(&commit, repo, &oid)){
            fprintf(stderr, "Failed to lookup the next object\n");
            return 1;
        }
        parse_commit_tree(repo,commit);
        git_commit_free(commit);
    }
    git_revwalk_free(walker);
}


int read_patterns_dir(void){
    fs::path patterns_dir;
    patterns_dir += fs::current_path();
    patterns_dir += "/patterns";
    for (auto & p : fs::directory_iterator(patterns_dir)) {
        std::ifstream jsonfile(p.path().string());
        json j;
        jsonfile >> j;
        for (json::iterator it = j.begin(); it != j.end(); ++it) {
            (it.value()["type"] == "secretFilename" ?
             filenames_to_find : content_to_find)
                    .push_back(it.value());
        }
    }
}

int main(int argc, char *argv[]) {
    /*
    int error = 0;
    git_libgit2_init();
    
    error = git_clone(&repo,"https://github.com/pegleg2060/PwnDelorean.git" , OUTPUT_DIR, NULL);
    if (error != 0) {
        print_git_error();
        exit(1);
    }
    printf("Git repo saved at: %s\n", git_repository_path(repo));
    test(repo);
    git_repository_free(repo);
    system("rm -r /tmp/yay");
     */
    static plog::ColorConsoleAppender<plog::TxtFormatter> consoleAppender;
    plog::init(plog::verbose, &consoleAppender);
    LOG_VERBOSE << "This is a VERBOSE message";
    /*
    read_patterns_dir();
    for (auto it = filenames_to_find.begin(); it != filenames_to_find.end(); ++it) {
        cout << *it << "\n";
    }
     */
    return 0;
}

