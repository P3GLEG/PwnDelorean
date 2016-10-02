#include "../libgit2/include/git2.h"
#include "../libgit2/include/git2/clone.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef _WIN32
# include <pthread.h>
# include <unistd.h>
#endif
#include <iostream>

void printGitError(){
    printf("ERROR: %s\n", giterr_last()->message);
}

void cloneRepo(git_repository** repo){


}

void getAllFiles(git_repository* repo){
  int error = 0;
  git_revwalk *walk;
  git_commit *wcommit = NULL;
  git_oid oid;
  const git_signature *cauth;
  const char *cmsg;
  git_revwalk_new(&walk, repo);
  git_revwalk_sorting(walk, GIT_SORT_TOPOLOGICAL | GIT_SORT_REVERSE);
  while ((git_revwalk_next(&oid, walk)) == 0) {
	  error = git_commit_lookup(&wcommit, repo, &oid);
	  printGitError();
	  cmsg  = git_commit_message(wcommit);
	  cauth = git_commit_author(wcommit);
	  printf("%s (%s)\n", cmsg, cauth->email);
	  git_commit_free(wcommit);
  }
  git_revwalk_free(walk);


}

void refWalk(git_repository *repo){
  git_strarray ref_list;
  git_reference_list(&ref_list, repo);
  const char *refname;
  git_reference *ref;
  int i = 0;
  char *out = new char[1024]();
  for (i = 0; i < ref_list.count; ++i) {
    refname = ref_list.strings[i];
    git_reference_lookup(&ref, repo, refname);
    switch (git_reference_type(ref)) {
    case GIT_REF_OID:
      git_oid_fmt(out, git_reference_target(ref));
      printf("%s [%s]\n", refname, out);
      break;

    case GIT_REF_SYMBOLIC:
      printf("%s => %s\n", refname, git_reference_symbolic_target(ref));
      break;
    default:
      fprintf(stderr, "Unexpected reference type\n");
      exit(1);
    }
  }
  git_strarray_free(&ref_list);
  free(out);
}

int main(void){
    int error = 0;
    git_libgit2_init();
    git_repository *repo = NULL;
    error = git_clone(&repo,"https://github.com/pegleg2060/PwnDelorean.git" , "/tmp/yay", NULL);
    if (error != 0) {
        printGitError();
        exit(1);
    }
    printf("Git repo saved at: %s\n", git_repository_path(repo));
    refWalk(repo);
    git_repository_free(repo);
    return 0;
        
}


