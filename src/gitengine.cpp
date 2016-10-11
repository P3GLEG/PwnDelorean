#include <deque>
#include "gitengine.h"
#include "../deps/libgit2/include/git2/diff.h"

namespace fs = std::experimental::filesystem;
git_repository *repo = NULL;

std::map<std::string, bool> blobs;
Engine* e;
GitEngine::GitEngine(void) {
git_libgit2_init();
    this->engine.Init();
    e = &this->engine;
}

void print_git_error() {
    LOG_ERROR << giterr_last()->message;
}

void parse_blob(const git_blob *blob, std::string filename, std::string oid, std::string root_path) {
    std::string temp((const char *) git_blob_rawcontent(blob), (size_t) git_blob_rawsize(blob));
    if (!temp.empty()) {
        std::stringstream ss(temp);
        std::string line;
        int line_number = 0;
        root_path+=filename;
        while (std::getline(ss, line, '\n')) {
            e->search_for_content_match(line ,line_number, root_path, oid);
            line_number++;
        }
    }
}

void parse_tree_entry(const char* root_path, const git_tree_entry *entry) {
    git_otype type = git_tree_entry_type(entry);
    switch (type) {
        case GIT_OBJ_BLOB: {
            git_object *blob;
            char oidstr[GIT_OID_HEXSZ + 1];
            git_oid_tostr(oidstr, sizeof(oidstr), git_tree_entry_id(entry));
            git_tree_entry_to_object(&blob, repo, entry);
            std::string filename(git_tree_entry_name(entry));
            if (blobs[oidstr]) { // We can assume that the SHA1 hash has already been found previously
                LOG_DEBUG << "Already parsed file: " << filename << oidstr;
            } else {
                e->search_for_filename_match(filename, oidstr, root_path);
                if(git_blob_is_binary((const git_blob *)blob) == 1){
                    LOG_DEBUG << "Binary file found:" << filename;
                }else{
                    parse_blob((const git_blob *) blob, filename, oidstr, root_path);
                }
                blobs[oidstr] = true;
            }
        }
            break;
        default:
            //TODO: Handle other types if need be
            break;
    }
}

int tree_walk_cb(const char *root, const git_tree_entry *entry, void *payload) {
    parse_tree_entry(root, entry);
    return SUCCESS;
}

int parse_commit_tree(git_repository *repo, const git_commit *commit) {
    int error = 0;
    git_tree *tree = NULL;
    error = git_tree_lookup(&tree, repo, git_commit_tree_id(commit));
    if (error != 0) {
        print_git_error();
        return FAILURE;
    }
    error = git_tree_walk(tree, GIT_TREEWALK_PRE, tree_walk_cb, NULL);
    if (error != 0) {
        print_git_error();
        return FAILURE;
    }
    return SUCCESS;
}

int begin_rev_traverse(const char* head_rev){
    git_oid oid;
    git_revwalk *walker;
    git_commit *commit;
    if (git_oid_fromstr(&oid, head_rev) != 0) {
        fprintf(stderr, "Invalid git object: '%s'\n", head_rev);
        return FAILURE;
    }
    git_revwalk_new(&walker, repo);
    git_revwalk_sorting(walker, GIT_SORT_TOPOLOGICAL);
    git_revwalk_push(walker, &oid);
    while (git_revwalk_next(&oid, walker) == 0) {
        if (git_commit_lookup(&commit, repo, &oid)) {
            fprintf(stderr, "Failed to lookup the next object\n");
        }
        parse_commit_tree(repo, commit);
        git_commit_free(commit);
    }
    git_revwalk_free(walker);
}

int traverse_head_branch(std::string head_filepath){
    FILE *head_fileptr;
    char head_rev[41];
    if ((head_fileptr = fopen(head_filepath.c_str(), "r")) == NULL) {
        LOG_ERROR << "Error opening " << head_filepath.c_str();
        return FAILURE;
    }
    if (fread(head_rev, 40, 1, head_fileptr) != 1) {
        LOG_ERROR << "Error reading from " << head_filepath.c_str();
        fclose(head_fileptr);
        return FAILURE;
    }
    fclose(head_fileptr);
    LOG_DEBUG << "head rev set to :" << head_rev;
    //TODO: Test threading here
    begin_rev_traverse(head_rev);
    return SUCCESS;
}
int begin(const char *local_repo_dir) {

    std::vector<std::string> branche_head_revs;
    LOG_DEBUG << "Git repo saved at: " << git_repository_path(repo);
   //TODO: Add different refs
    fs::path remote_refs(git_repository_path(repo));
    remote_refs /= "refs/remotes/";
    for(auto& path: fs::directory_iterator(remote_refs)){
           LOG_DEBUG << "Remote branch found :" << path ;
        for(auto& rev_file: fs::directory_iterator(path.path())){
            LOG_DEBUG << "Branch filename found :" << rev_file;
            traverse_head_branch(rev_file.path());
        }
    }
    e->output_matches();
    git_repository_free(repo);
    git_libgit2_shutdown();
    return SUCCESS;
}

int GitEngine::local_start(const char *repo_location){
    int error = 0;
    error = git_repository_open(&repo, repo_location);
    if (error != 0) {
        LOG_FATAL << "Unable to find locat repository at :" << repo_location;
        return FAILURE;
    }
    LOG_DEBUG << "Opened local repository at :" << repo_location;
    begin(repo_location);
    return SUCCESS;
}

int GitEngine::remote_start(const char *url,const char *clone_dir) {
    int error = 0;
    error = git_clone(&repo, url, clone_dir, NULL);
    if (error != 0) {
        //TODO: Prompt to continue if output_dir has repo already
        LOG_FATAL << "Unable to clone repo due to network error! Or previously been cloned into " << clone_dir;
        return FAILURE;
    }
    return begin(clone_dir);
}


 





