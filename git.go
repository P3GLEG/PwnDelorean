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

func GetRepoFilenames(repoUrl string) ([]GitFile, error){
	var filenames []GitFile
	dir, err := ioutil.TempDir("", "PwnDelorean")
	if err != nil {
		return filenames, err
	}
	defer os.RemoveAll(dir)
	repo, err := git.Clone(repoUrl, dir, &git.CloneOptions{})
	if err != nil {
		return filenames, err
	}
	//TODO: Different Branches
	head, err := repo.Head()
	if err != nil{
		return filenames, err
	}
	walk, err := repo.Walk()
	if err != nil{
		return filenames, err
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
				commit.Id().String()
				filenames = append(filenames, GitFile{te.Name,
					td, commit.Id().String()})
			}
			return 0
		})
	}
	return filenames, nil
}



