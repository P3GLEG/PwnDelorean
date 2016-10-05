/*
 * @Author Paul Ganea
 */
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
#include <map>
#include <fstream>
#include "../deps/json/src/json.hpp"
#include <plog/Log.h>
#include <plog/Appenders/ColorConsoleAppender.h>
#include <re2.h>
#include <re2/set.h>

#define SUCCESS 0
#define FAIL -1
#define OUTPUT_DIR "/tmp/yay/"
#define REPO OUTPUT_DIR ".git"

using json = nlohmann::json;
using namespace std;
namespace fs = std::experimental::filesystem;

RE2::Set filenames_regexes(RE2::DefaultOptions, RE2::UNANCHORED);
RE2::Set content_regexes(RE2::DefaultOptions, RE2::UNANCHORED);
git_repository *repo = NULL;
map <string, bool> blobs;
map <string, bool> filename_matches;
map <string, bool> content_matches;

string *error_holder = NULL;

bool search_for_content_match(string line){
    if(content_matches.count(line) > 0){
        //Already found this no need to do work again
        LOG_DEBUG << "Attempted to parse " << line << " again";

        return true;
    }
    vector<int> matched_regexes;
    content_regexes.Match(line, &matched_regexes);
        if( matched_regexes.size() != 0) {
            for(auto i: matched_regexes){
                LOG_ERROR <<i;
            }
            content_matches[line] = true;
            //Fast finsh, if you want to add which regex was matched do it here
            return true;
        }

    return false;
}

bool search_for_filename_match(string filename){
    if(filename_matches.count(filename) >0){
        //Already found this no need to do work again
        LOG_DEBUG << "Attempted to parse " << filename << " again";
        return true;
    }
    vector<int> matched_regexes;
    filenames_regexes.Match(filename, &matched_regexes);
    if( matched_regexes.size() != 0) {
        for(auto i: matched_regexes){
                LOG_ERROR <<i;
        }
        filename_matches[filename] = true;
        //Fast finish, if you want to add which regex was matched do it here
        return true;
    }
    return false;
}

void parse_blob(const git_blob *blob)
{
    string temp((const char*) git_blob_rawcontent(blob), (size_t)git_blob_rawsize(blob));
    if (!temp.empty()){
        std::stringstream ss(temp);
        std::string line;
        while(std::getline(ss,line,'\n')){
            search_for_content_match(line);
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
                LOG_DEBUG << "Already parsed file: " << filename << oidstr;
            } else {
                //TODO: Add oidstr to output
                parse_blob((const git_blob *) blob);
                search_for_filename_match(filename);
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
    return SUCCESS;
}

int parse_commit_tree(git_repository *repo, const git_commit *commit){
    int error = 0;
    git_tree *tree = NULL;
    error = git_tree_lookup(&tree, repo, git_commit_tree_id(commit));
    if(error != 0){
        print_git_error();
        return FAIL;
    }
    error = git_tree_walk(tree, GIT_TREEWALK_PRE, tree_walk_cb, NULL);
        if(error != 0){
        print_git_error();
        return FAIL;
    }
    return SUCCESS;
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
        return FAIL;
    }
    if(fread(head_rev, 40, 1, head_fileptr) != 1){
        fprintf(stderr, "Error reading from '%s'\n", head_filepath);
        fclose(head_fileptr);
        return FAIL;
    }
    fclose(head_fileptr);
    git_oid oid;
    git_revwalk *walker;
    git_commit *commit;

    if(git_oid_fromstr(&oid, head_rev) != 0){
        fprintf(stderr, "Invalid git object: '%s'\n", head_rev);
        return FAIL;
    }

    git_revwalk_new(&walker, repo);
    git_revwalk_sorting(walker, GIT_SORT_TOPOLOGICAL);
    git_revwalk_push(walker, &oid);
    while(git_revwalk_next(&oid, walker) == 0) {
        if(git_commit_lookup(&commit, repo, &oid)){
            fprintf(stderr, "Failed to lookup the next object\n");
            return FAIL;
        }
        parse_commit_tree(repo,commit);
        git_commit_free(commit);
    }
    for(auto i: filename_matches){
        LOG_VERBOSE << "Filename match: " << i.first;
    }
    for(auto i: content_matches){
        LOG_VERBOSE << "Content match: " << i.first;
    }
    git_revwalk_free(walker);
    return SUCCESS;
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
            if(it.value()["type"] == "secretFilename"){
                LOG_DEBUG << "Filename Regex : " << regex;
                filenames_regexes.Add(regex, error_holder);
            }
            else {
                LOG_DEBUG << "Content Regex : " << regex;
                content_regexes.Add(regex, error_holder);
            }
        }
        jsonfile.close();

    }
    if (!filenames_regexes.Compile() | !content_regexes.Compile()){
        LOG_ERROR << "Error while compiling regexes";
        return FAIL;
    }
    return SUCCESS;
}

int init(void) {
    git_libgit2_init();
    static plog::RollingFileAppender<plog::CsvFormatter> fileAppender("debug.txt", 100000000, 3);
    static plog::ColorConsoleAppender<plog::TxtFormatter> consoleAppender;
    plog::init(plog::verbose, &consoleAppender).addAppender(&fileAppender);
    if(read_patterns_dir() != 0){
        return FAIL;
    }
    return SUCCESS;
}

int main(int argc, char *argv[]) {
    if (init() != 0) {
        LOG_ERROR << "Failed to initialize the program";   
        exit(FAIL);
    }
    int error = 0;
    error = git_clone(&repo,"https://github.com/pegleg2060/PwnDelorean.git" , OUTPUT_DIR, NULL);
    if (error != 0) {
        print_git_error();
        exit(FAIL);
    }
    LOG_VERBOSE <<"Git repo saved at: "<< git_repository_path(repo);
    start(repo);
    git_repository_free(repo);
    system("rm -r /tmp/yay");
    return SUCCESS;
}

