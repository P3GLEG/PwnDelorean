package main

import (
	"gopkg.in/libgit2/git2go.v25"
	"io/ioutil"
	"os"
	"bytes"
)

type GitFile struct {
	Name string
	Filepath string
	CommitId string
}

func searchContents(contents []byte) (bool, Pattern){
	for _, pattern := range fileContentLiterals{
		if bytes.Contains([]byte(pattern.Value), contents){
			return true, pattern
		}
	}
	for _, pattern := range secretFileNameRegexes {
		if pattern.Regex.Match(contents){
			return true, pattern
		}
	}
	return false, Pattern{}
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
	//TODO: Different Branches
	head, err := repo.Head()
	if err != nil{
		return filenames, matches, err
	}
	walk, err := repo.Walk()
	if err != nil{
		return filenames, matches, err
	}
	walk.Sorting(git.SortTopological)
	walk.Push(head.Target())
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
						match, pattern := searchContents(blob.Contents())
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



