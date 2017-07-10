package main

import (
	"gopkg.in/libgit2/git2go.v25"
	"io/ioutil"
	"os"
	"errors"
	"encoding/json"
	"net/http"
	"fmt"
	"strings"
)

type GitFile struct {
	Name string
	Filepath string
	CommitId string
	RepoUrl string
}

const gitHubApiVersion = "application/vnd.github.v3+json"
const INVALID_PLAINTEXT_CREDS = -10
const INVALID_SSH_CREDS= -11

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


func credentialsCallback(url string, username string, allowedTypes git.CredType) (git.ErrorCode, *git.Cred) {
	git_user := os.Getenv("GIT_USER")
	git_pass := os.Getenv("GIT_PASS")
	git_priv := os.Getenv("GIT_PRIV_KEY")
	git_pub := os.Getenv("GIT_PUB_KEY")
	git_passphrase := os.Getenv("GIT_PASSPHRASE")
	switch *useGitCredentials{
	case "ssh":
		if len(git_user) == 0 || len(git_pub) ==0 || len(git_priv) == 0{
			return git.ErrorCode(INVALID_SSH_CREDS), nil
		}
		ret, cred := git.NewCredSshKey(git_user,
		git_pub,
		git_priv,
		git_passphrase)
		return git.ErrorCode(ret), &cred
	case "plaintext":
		if len(git_user) == 0 || len(git_pass) == 0{
			return git.ErrorCode(INVALID_PLAINTEXT_CREDS), nil
		}
		ret, cred := git.NewCredUserpassPlaintext(git_user, git_pass)
		return git.ErrorCode(ret), &cred
	default:
		return git.ErrorCode(-1), nil
	}
}

func certificateCheckCallback(cert *git.Certificate, valid bool, hostname string) git.ErrorCode {
	//TODO: Fix lol
	return 0
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
	files, matches, err := getRepoFilenames(repoUrl)
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

func getRepoFilenames(repoUrl string) ([]GitFile, []*Match, error){
	var table = make(map[string]*Match)
	var filenames []GitFile
	var matches []*Match
	dir, err := ioutil.TempDir("", "PwnDelorean")
	if err != nil {
		return filenames, matches, err
	}
	defer os.RemoveAll(dir)
	cloneOptions := &git.CloneOptions{}
	if len(*useGitCredentials) !=0 {
		cloneOptions.FetchOptions = &git.FetchOptions{
			RemoteCallbacks: git.RemoteCallbacks{
				CredentialsCallback:      credentialsCallback,
				CertificateCheckCallback: certificateCheckCallback,
			},
		}
	}
	repo, err := git.Clone(repoUrl, dir, cloneOptions)
	if err != nil {
		if err.(*git.GitError).Code == INVALID_PLAINTEXT_CREDS{
			return filenames, matches,
				errors.New("Please check your GIT_USER and GIT_PASS Environment variables")
		} else if len(*useGitCredentials) ==0 {
			return filenames, matches, errors.New(
				"-creds flag not in use, the repo may be private or doesn't exist")
		} else {
			return filenames, matches, errors.New("Git repo appears to not exist")
		}
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



func appendGitMatch(pattern Pattern, filename GitFile, table map[string]*Match) {
	//You can't assume a file hasn't moved directories in history
	// which is why you use the filepath + filename for uniqueness
	path := filename.Filepath
	if len(path) == 0 {
		path = "/" + filename.Name
	} else {
		path += filename.Name
	}
	_, exists := table[path]
	if exists {
		table[path].CommitIds = append(table[path].CommitIds, filename.CommitId)
	} else {
        //TODO: Line matched
		table[path] = &Match{filename.Name, path,
							 []string{filename.CommitId}, pattern.Description,
			pattern.Value, ""}
		fmt.Println(fmt.Sprintf("Found match %s %s %s %s", pattern.Description,
			path, filename.RepoUrl, pattern.Value))
	}
}

func gitSecretFilenameLiteralSearch(files []GitFile) []*Match {
	var table = make(map[string]*Match)
	for _, pattern := range secretFileNameLiterals {
		for _, filename := range files {
			if strings.Contains(filename.Name, pattern.Value) {
				appendGitMatch(pattern, filename, table)
			}
		}
	}
	var results []*Match
	for _, values := range table {
		results = append(results, values)
	}
	return results
}

func gitSecretFilenameRegexSearch(files []GitFile) []*Match {
	var table = make(map[string]*Match)
	for _, filename := range files {
		for _, pattern := range secretFileNameRegexes {
			if pattern.Regex.MatchString(filename.Name) {
				appendGitMatch(pattern, filename, table)
				break
			}
		}
	}
	var results []*Match
	for _, values := range table {
		results = append(results, values)
	}
	return results
}

func outputCSVGitRepo(matches []*Match){
	records := [][]string{{"Filename", "Description", "Filepath", "CommitID", "Value"}}
	for _, match := range matches {
		records = append(records, []string{match.Filename, match.Description, match.Filepath,
										   strings.Join(match.CommitIds, "|"), match.Value})
	}
	outputCSV(*outputCSVFlag, records)
}

func startGitRepoScan(){
	results, err := gitRepoSearch(*repoToScanFlag)
		if err != nil{
			fmt.Println(err)
		}
		if len(*outputCSVFlag) != 0 {
			outputCSVGitRepo(results)
		}
}

func startGitOrganizationScan(){
	var results = []*Match{}
		urls, err := getAllOrganizationsRepoUrls(*organizationFlag)
		if err!= nil {
			fmt.Println(err)
			os.Exit(-1)
		}
		matchChan := make(chan []*Match, len(urls))
		for _, url := range urls{
			go getOrgRepo(url, matchChan)
		}
		for i:= 0; i < len(urls); i++{
			select {
			case matches := <-matchChan:
				results = append(results, matches...)
			}
		}
		if len(*outputCSVFlag) != 0 {
			outputCSVGitRepo(results)
		}
}
