package main

import (
	"gopkg.in/libgit2/git2go.v25"
	"io/ioutil"
	"os"
)

type GitFile struct {
	Name string
	Filepath string
	CommitId string
}

func searchBlobContents(contents []byte) (bool, Pattern){
	/*
	This is matching on things it shouldn't
	for _, pattern := range fileContentLiterals{
		if bytes.Contains([]byte(pattern.Value), contents){
			fmt.Println(pattern.Value)
			fmt.Println(string(contents))
			return true, pattern
		}
	}
	*/
	for _, pattern := range secretFileNameRegexes {
		if pattern.Regex.Match(contents){
			return true, pattern
		}
	}
	return false, Pattern{}
}

func searchAllBranches(repo *git.Repository, walk *git.RevWalk) error{
	itr, err := repo.NewBranchIterator(git.BranchRemote)
	defer itr.Free()
	if err != nil{
		return err
	}
	var f = func(b *git.Branch, t git.BranchType) error {
		walk.Push(b.Target())
		return nil
	}
	itr.ForEach(f)
	return nil
}

func GetRepoFilenames(repoUrl string) ([]GitFile, []*Match, error){
	var table = make(map[string]*Match)
	var filenames []GitFile
	var matches []*Match
	dir, err := ioutil.TempDir("", "PwnDelorean")
	if err != nil {
		return filenames, matches, err
	}
	defer os.RemoveAll(dir)
	repo, err := git.Clone(repoUrl, dir, &git.CloneOptions{})
	if err != nil {
		return filenames, matches, err
	}
	defer repo.Free()
	head, err := repo.Head()
	if err != nil{
		return filenames, matches, err
	}
	defer head.Free()
	walk, err := repo.Walk()
	if err != nil{
		return filenames, matches, err
	}
	defer walk.Free()
	walk.Sorting(git.SortTopological)
	walk.Push(head.Target())
	searchAllBranches(repo, walk)
	gi := head.Target()
	for {
		err := walk.Next(gi)
		if err != nil {
			break
		}
		commit, err := repo.LookupCommit(gi)
		tree, err := commit.Tree()
		err = tree.Walk(func(td string, te *git.TreeEntry) int {
			if te.Type == git.ObjectBlob && te.Filemode == git.FilemodeBlob{
				gitFile := GitFile{te.Name,
					td, commit.Id().String()}
				if !*fileNamesOnlyFlag {
					blob, err := repo.LookupBlob(te.Id)
					if err == nil {
						match, pattern := searchBlobContents(blob.Contents())
						if match {
							appendGitMatch(pattern, gitFile, table)
						}
					}
				}
				filenames = append(filenames, gitFile)
			}
			return 0
		})
	}
	var results []*Match
	for _, values := range table {
		results = append(results, values)
	}
	return filenames, results, nil
}



