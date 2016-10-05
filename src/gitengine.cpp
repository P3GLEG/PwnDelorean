#include "gitengine.h"
#include "engine.h"


Engine engine;
git_repository *repo = NULL;
std::map<std::string, bool> blobs;

void print_git_error() {
    LOG_ERROR << giterr_last()->message;
}

void parse_blob(const git_blob *blob) {
    std::string temp((const char *) git_blob_rawcontent(blob), (size_t) git_blob_rawsize(blob));
    if (!temp.empty()) {
        std::stringstream ss(temp);
        std::string line;
        while (std::getline(ss, line, '\n')) {
            engine.search_for_content_match(line);
        }
    }
}

void parse_tree_entry(const git_tree_entry *entry) {
    git_otype type = git_tree_entry_type(entry);
    switch (type) {
        case GIT_OBJ_BLOB: {
            git_object *blob;
            char oidstr[GIT_OID_HEXSZ + 1];
            git_oid_tostr(oidstr, sizeof(oidstr), git_tree_entry_id(entry));
            git_tree_entry_to_object(&blob, repo, entry);
            std::string filename(git_tree_entry_name(entry));
            if (blobs[oidstr]) {
                LOG_DEBUG << "Already parsed file: " << filename << oidstr;
            } else {
                //TODO: Add oidstr to output
                parse_blob((const git_blob *) blob);
                engine.search_for_filename_match(filename);
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
    parse_tree_entry(entry);
    return SUCCESS;
}

int parse_commit_tree(git_repository *repo, const git_commit *commit) {
    int error = 0;
    git_tree *tree = NULL;
    error = git_tree_lookup(&tree, repo, git_commit_tree_id(commit));
    if (error != 0) {
        print_git_error();
        return FAIL;
    }
    error = git_tree_walk(tree, GIT_TREEWALK_PRE, tree_walk_cb, NULL);
    if (error != 0) {
        print_git_error();
        return FAIL;
    }
    return SUCCESS;
}

int GitEngine::start(std::string url) {
    int error = 0;
    git_libgit2_init();
    engine.Init();
    error = git_clone(&repo, url.c_str(), OUTPUT_DIR, NULL);
    if (error != 0) {
        print_git_error();
        exit(FAIL);
    }
    LOG_VERBOSE << "Git repo saved at: " << git_repository_path(repo);
    char head_filepath[512];
    FILE *head_fileptr;
    char head_rev[41];
    strcpy(head_filepath, REPO);
    //TODO: Add different refs
    if (strrchr(REPO, '/') != (REPO + strlen(REPO)))
        strcat(head_filepath, "/refs/heads/master");
    else
        strcat(head_filepath, "refs/heads/master");

    if ((head_fileptr = fopen(head_filepath, "r")) == NULL) {
        fprintf(stderr, "Error opening '%s'\n", head_filepath);
        return FAIL;
    }
    if (fread(head_rev, 40, 1, head_fileptr) != 1) {
        fprintf(stderr, "Error reading from '%s'\n", head_filepath);
        fclose(head_fileptr);
        return FAIL;
    }
    fclose(head_fileptr);
    git_oid oid;
    git_revwalk *walker;
    git_commit *commit;

    if (git_oid_fromstr(&oid, head_rev) != 0) {
        fprintf(stderr, "Invalid git object: '%s'\n", head_rev);
        return FAIL;
    }

    git_revwalk_new(&walker, repo);
    git_revwalk_sorting(walker, GIT_SORT_TOPOLOGICAL);
    git_revwalk_push(walker, &oid);
    while (git_revwalk_next(&oid, walker) == 0) {
        if (git_commit_lookup(&commit, repo, &oid)) {
            fprintf(stderr, "Failed to lookup the next object\n");
            return FAIL;
        }
        parse_commit_tree(repo, commit);
        git_commit_free(commit);
    }
    for (auto i: engine.filename_matches) {
        LOG_VERBOSE << "Filename match: " << i.first;
    }
    for (auto i: engine.content_matches) {
        LOG_VERBOSE << "Content match: " << i.first;
    }
    git_revwalk_free(walker);
    git_repository_free(repo);
    git_libgit2_shutdown();
    return SUCCESS;
}

 





