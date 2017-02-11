#include <deque>
#include "gitengine.h"

namespace fs = std::experimental::filesystem;
git_repository *repo = NULL;

std::map<std::string, bool> blobs;
Engine* e;
GitEngine::GitEngine(void) {
git_libgit2_init();
    this->engine.Init();
    e = &this->engine;
}

typedef struct progress_data {
    git_transfer_progress fetch_progress;
    size_t completed_steps;
    size_t total_steps;
    const char *path;
} progress_data;

void print_git_error() {
    LOG_ERROR << giterr_last()->message;
}

static void print_progress(const progress_data *pd)
{
	int network_percent = pd->fetch_progress.total_objects > 0 ?
		(100*pd->fetch_progress.received_objects) / pd->fetch_progress.total_objects :
		0;
	int index_percent = pd->fetch_progress.total_objects > 0 ?
		(100*pd->fetch_progress.indexed_objects) / pd->fetch_progress.total_objects :
		0;

	int checkout_percent = (int) (pd->total_steps > 0
            ? (100 * pd->completed_steps) / pd->total_steps
            : 0);
	int kbytes = (int) (pd->fetch_progress.received_bytes / 1024);

	if (pd->fetch_progress.total_objects &&
		pd->fetch_progress.received_objects == pd->fetch_progress.total_objects) {
		printf("Resolving deltas %d/%d\r",
		       pd->fetch_progress.indexed_deltas,
		       pd->fetch_progress.total_deltas);
        fflush(stdout);
	} else {
		printf("Clone progress %3d%% (%4d kb, %5d/%5d)  /  idx %3d%% (%5d/%5d)  %s\r",
		   network_percent, kbytes,
		   pd->fetch_progress.received_objects, pd->fetch_progress.total_objects,
		   index_percent, pd->fetch_progress.indexed_objects, pd->fetch_progress.total_objects,
		   pd->path);
	}
}

static int sideband_progress(const char *str, int len, void *payload)
{
	printf("remote: %*s", len, str);
	fflush(stdout);
	return 0;
}

static int fetch_progress(const git_transfer_progress *stats, void *payload)
{
	progress_data *pd = (progress_data*)payload;
	pd->fetch_progress = *stats;
	print_progress(pd);
	return 0;
}
static void checkout_progress(const char *path, size_t cur, size_t tot, void *payload)
{
	progress_data *pd = (progress_data*)payload;
	pd->completed_steps = cur;
	pd->total_steps = tot;
	pd->path = path;
	print_progress(pd);
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
            git_object_free(blob);

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
    git_tree_free(tree);
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
    git_repository_free(repo);
    git_libgit2_shutdown();
    return SUCCESS;
}

int GitEngine::remote_start(const char *url,const char *clone_dir) {
    int error = 0;
    progress_data pd = {{0}};
	git_clone_options clone_opts = GIT_CLONE_OPTIONS_INIT;
	git_checkout_options checkout_opts = GIT_CHECKOUT_OPTIONS_INIT;
	git_proxy_options proxy_opts =  GIT_PROXY_OPTIONS_INIT;
	checkout_opts.checkout_strategy = GIT_CHECKOUT_SAFE;
	checkout_opts.progress_cb = checkout_progress;
	checkout_opts.progress_payload = &pd;
	clone_opts.checkout_opts = checkout_opts;
	clone_opts.fetch_opts.callbacks.sideband_progress = sideband_progress;
	clone_opts.fetch_opts.callbacks.transfer_progress = &fetch_progress;
	clone_opts.fetch_opts.callbacks.payload = &pd;
    error = git_clone(&repo, url, clone_dir, &clone_opts);
    if (error != 0) {
        //TODO: Prompt to continue if output_dir has repo already
        LOG_FATAL << "Unable to clone repo due to network error! Or previously been cloned into " << clone_dir;
	LOG_FATAL << "Error: " << giterr_last()->message;
        return FAILURE;
    }
    begin(clone_dir);
    git_repository_free(repo);
    git_libgit2_shutdown();
    return SUCCESS;
}



 





