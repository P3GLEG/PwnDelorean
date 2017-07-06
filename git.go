package main

import (
	"gopkg.in/libgit2/git2go.v25"
	"io/ioutil"
	"os"
	"errors"
	"encoding/json"
	"net/http"
	"fmt"
)

type GitFile struct {
	Name string
	Filepath string
	CommitId string
	RepoUrl string
}

const gitHubApiVersion = "application/vnd.github.v3+json"
func gitHubPerformRequest(urlString string) ([]string, error) {
	var urls = []string{}
	client := &http.Client{}
	req, err := http.NewRequest("GET", urlString, nil)
	if err != nil {
		return urls, err
	}
	req.Header.Set("Accept", gitHubApiVersion)
	//TODO: Add auth here
	//req.SetBasicAuth(Github.Username, Github.AccessToken)
	resp, err := client.Do(req)
	if err != nil {
		return urls, err
	}
	defer resp.Body.Close()
	body, _ := ioutil.ReadAll(resp.Body)
	switch resp.StatusCode {
	case 200:
		var data []interface{}
		err := json.Unmarshal(body, &data)
		if err != nil {
			return urls, err
		}
		for _, repo := range data{
			url := repo.(map[string]interface{})["html_url"]
			urls = append(urls, url.(string))
		}
		return urls, nil
	case 400:
		return urls, errors.New("Bad request sent to Github")
	case 422:
		return urls, errors.New("Invalid fields sent to Github")
	case 403:
		return urls, errors.New("Github API Rate Limiting detected")
	case 404:
		return urls, errors.New("Organization doesn't exist")
	default:
		return urls, errors.New(string(body))
	}
	return urls, nil
}

func getAllOrganizationsRepoUrls(orgName string) ([]string, error){
	urlString := fmt.Sprintf("https://api.github.com/orgs/%s/repos",
		orgName)
	if *ignoreForkRepos{
		urlString+="?type=sources" //Read Github API Documentation for details
	}
	urls, err := gitHubPerformRequest(urlString)
	if err!= nil {
		return nil, err
	}
	return urls, err
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
	for _, pattern := range fileContentRegexes{
		if pattern.Regex.Match(contents){
			return true, pattern
		}
	}
	return false, Pattern{}
}

func gitRepoSearch(repoUrl string) ([]*Match, error){
	var results = []*Match{}
	files, matches, err := GetRepoFilenames(repoUrl)
	if err != nil {
			return results, err
	}
	results = append(results, gitSecretFilenameLiteralSearch(files)...)
	results = append(results, gitSecretFilenameRegexSearch(files)...)
	results = append(results, matches...)
	return results, nil
}

func getOrgRepo(url string, matchChan chan[]*Match){
	matches, err := gitRepoSearch(url)
	if err != nil{
		fmt.Println(err)
	}
	matchChan <- matches
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
					td, commit.Id().String(), repoUrl}
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



