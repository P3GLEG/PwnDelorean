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
#include <dirent.h>
#include <unordered_map>

#define OUTPUT_DIR "/tmp/yay/"
#define REPO OUTPUT_DIR ".git"

using namespace std;
git_repository *repo = NULL;
unordered_map <string, bool> blobs;

static void parse_blob(const git_blob *blob)
{
        fwrite(git_blob_rawcontent(blob), (size_t)git_blob_rawsize(blob), 1, stdout);
}

void print_git_error(){
    printf("ERROR: %s\n", giterr_last()->message);
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
                cout << "Already exists" <<endl;
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
    const char *name = git_tree_entry_name(entry);
    git_otype type = git_tree_entry_type(entry);
    parse_tree_entry(entry);
    printf("Filename: %s \n", name);
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


int test(git_repository *repo){
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

int cred_acquire_cb(git_cred **out,const char * url,const char * username_from_url,unsigned int allowed_types,void * payload)
{
	char username[128] = {0};
	char password[128] = {0};

	printf("Username: ");
	scanf("%s", username);

	printf("Password: ");
	scanf("%s", password);

	return git_cred_userpass_plaintext_new(out, username, password);
}


int main(int argc, char *argv[]){
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
    return 0;
        
}
