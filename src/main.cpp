extern "C" {
    #include "../deps/libgit2/include/git2.h"
    #include "../deps/libgit2/include/git2/clone.h"
    #include <stdio.h>
    #include <string.h>
}
#ifndef _WIN32
    # include <pthread.h>
    # include <unistd.h>
#endif
#include <experimental/filesystem>
#include <iostream>
#include <unordered_map>
#include <fstream>
#include "../deps/json/src/json.hpp"
#include <plog/Log.h>
#include <plog/Appenders/ColorConsoleAppender.h>
#include <re2.h>
#include <re2/set.h>

#define OUTPUT_DIR "/tmp/yay/"
#define REPO OUTPUT_DIR ".git"

using json = nlohmann::json;
using namespace std;
namespace fs = std::experimental::filesystem;
RE2::Set filenames_regexes(RE2::DefaultOptions, RE2::UNANCHORED);
RE2::Set content_regexes(RE2::DefaultOptions, RE2::UNANCHORED);
git_repository *repo = NULL;
unordered_map <string, bool> blobs;
string *error_holder = NULL;
bool regex_match(string line, bool search_for_filename){
    vector<int> matched_regexes;
    if(search_for_filename){
        filenames_regexes.Match(line, &matched_regexes);
        if( matched_regexes.size() != 0) {
            for (auto i: matched_regexes) {
                LOG_VERBOSE << "Filename Found : " << line;
            }
            return true;
        }
    }else{
        content_regexes.Match(line, &matched_regexes);
        if( matched_regexes.size() != 0) {
            for (auto i: matched_regexes) {
                LOG_VERBOSE << "Match Found : " << line;
            }
            return true;
        }

    }
    return false;
}
void parse_blob(const git_blob *blob)
{
    string a((const char*) git_blob_rawcontent(blob), (size_t)git_blob_rawsize(blob));
    if (!a.empty()){
        std::stringstream ss(a);
        std::string line;
        while(std::getline(ss,line,'\n')){
            regex_match(line, false);
        }
    }
}

void print_git_error(){
    LOG_ERROR << giterr_last()->message;
}

void parse_tree_entry(const git_tree_entry *entry){
    git_otype type = git_tree_entry_type(entry);
    switch(type){
        case GIT_OBJ_BLOB: {
            git_object *blob;
            char oidstr[GIT_OID_HEXSZ + 1];
            git_oid_tostr(oidstr, sizeof(oidstr), git_tree_entry_id(entry));
            git_tree_entry_to_object(&blob, repo, entry);
            string filename(git_tree_entry_name(entry));
            if (blobs[oidstr]) {
                //LOG_DEBUG << "Already parsed file: " << filename << oidstr;
            } else {
                parse_blob((const git_blob *) blob);
                regex_match(filename, true);
                blobs[oidstr] = true;
            }
        }
            break;
        default:
            //TODO: Handle other types if need be
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
        //TODO: link description of secret found
        //However needed to compile regexes for speed
        for (json::iterator it = j.begin(); it != j.end(); ++it) {
            std::string regex = it.value()["regex"];
            (it.value()["type"] == "secretFilename" ?
             filenames_regexes: content_regexes)
                    .Add(regex, error_holder);
        }
        jsonfile.close();

    }
    if (!filenames_regexes.Compile() | !content_regexes.Compile()){
        LOG_ERROR << "Error while compiling regexes";
    }
}

void init(void) {
    git_libgit2_init();
    static plog::RollingFileAppender<plog::CsvFormatter> fileAppender("debug.txt", 8000, 3);
    static plog::ColorConsoleAppender<plog::TxtFormatter> consoleAppender;
    plog::init(plog::verbose, &consoleAppender).addAppender(&fileAppender);
    read_patterns_dir();
}

int main(int argc, char *argv[]) {
    init();
    int error = 0;
    error = git_clone(&repo,"https://github.com/pegleg2060/PwnDelorean.git" , OUTPUT_DIR, NULL);
    if (error != 0) {
        print_git_error();
        exit(1);
    }
    LOG_VERBOSE <<"Git repo saved at: "<< git_repository_path(repo);
    start(repo);
    git_repository_free(repo);
    system("rm -r /tmp/yay");
    return 0;
}

