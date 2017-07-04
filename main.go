package main

import (
	"fmt"
	"path/filepath"
	"os"
	"io/ioutil"
	"encoding/json"
	"flag"
	"regexp"
	"strings"
	"encoding/csv"
)

type Pattern struct {
	Description string `json:description`
	SecretType  string  `json:type`
	Value       string       `json:value`
	Regex       *regexp.Regexp
}

type Match struct {
	Filename    string
	Filepath    string
	CommitIds   []string
	Description string
	Value string
}

type FileStruct struct {
	Filename string
	Path string
}


var dirToScanFlag = flag.String("directory", "", "Directory to scan")
var repoToScanFlag = flag.String("url", "", "Git Repo URL to scan")
var outputCSVFlag = flag.String("csv", "", "Output in CSV Format")
var fileNamesOnlyFlag = flag.Bool("fileNamesOnly", false,
	"Disable searching through File for speed increase")
var organizationFlag= flag.String("organization", "", "Search all of an Organizations repos")
var ignoreForkRepos = flag.Bool("ignoreForkedRepos", true,
	"Ignore any Organization repos that are forked")

var secretFileNameLiterals = []Pattern{}
var secretFileNameRegexes = []Pattern{}
var fileContentLiterals = []Pattern{}
var fileContentRegexes = []Pattern{}

func initializePatterns(path string, info os.FileInfo, _ error) error {
	if !info.IsDir() {
		file, e := ioutil.ReadFile(path)
		if e != nil {
			fmt.Printf("File error: %v\n", e)
			os.Exit(1)
		}
		var data []Pattern
		json.Unmarshal(file, &data)
		for _, pattern := range data {
			switch pattern.SecretType {
			case "secretFilenameLiteral":
				secretFileNameLiterals = append(secretFileNameLiterals, pattern)
			case "secretFilenameRegex":
				pattern.Regex = regexp.MustCompile(pattern.Value)
				secretFileNameRegexes = append(secretFileNameRegexes, pattern)
			case "fileContentLiteral":
				fileContentLiterals = append(fileContentLiterals, pattern)
			case "fileContentRegex":
				pattern.Regex = regexp.MustCompile(pattern.Value)
				fileContentRegexes = append(fileContentRegexes, pattern)
			default:
				fmt.Println("Unable to read " + pattern.Description)
			}
		}
	}
	return nil
}


func GetAllFilesInDirectory(dir string) ([]FileStruct, error) {
	fileList := []FileStruct{}
	err := filepath.Walk(dir, func(path string, f os.FileInfo, err error) error {
		fileList = append(fileList, FileStruct{f.Name(), path})
		return nil
	})
	if err != nil{
		return nil, err
	}
	return fileList, nil
}

func appendFilesystemMatch(pattern Pattern, filestruct FileStruct, table map[string]*Match) {
	name := filestruct.Filename
	_, exists := table[name]
	if !exists {
		table[name] = &Match{filestruct.Filename, filestruct.Path,
			nil,pattern.Description, pattern.Value}
		fmt.Println(fmt.Sprintf("Found match %s: %s %s", pattern.Description, filestruct.Path, pattern.Value))
	}
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
		table[path] = &Match{filename.Name, path,
							 []string{filename.CommitId}, pattern.Description,
			pattern.Value}
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

func filesystemSecretFilenameLiteralSearch(files []FileStruct) []*Match {
	var table = make(map[string]*Match)
	for _, pattern := range secretFileNameLiterals {
		for _, filename := range files {
			if strings.Contains(filename.Filename, pattern.Value) {
				appendFilesystemMatch(pattern, filename, table)
			}
		}
	}
	var results []*Match
	for _, values := range table {
		results = append(results, values)
	}
	return results
}

func filesystemSecretFilenameRegexSearch(files []FileStruct) []*Match{
	var table = make(map[string]*Match)
	for _, filestruct := range files {
		for _, pattern := range secretFileNameRegexes {
			if pattern.Regex.MatchString(filestruct.Filename) {
				appendFilesystemMatch(pattern, filestruct, table)
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

func initalize() {
	err := filepath.Walk("./patterns", initializePatterns)
	if err != nil {
		fmt.Println(err)
	}
}

func outputCSV(filename string, records [][]string) {
	file, err := os.Create(filename)
	if err != nil{
		fmt.Println("Unable to open file, possibly due to permissions dump to STDOUT")
		w := csv.NewWriter(os.Stdout)
		w.WriteAll(records)
		if err := w.Error(); err != nil {
			fmt.Println(err)
		}
	}
	w := csv.NewWriter(file)
	w.WriteAll(records)
	defer file.Close()
}

func outputCSVFilesystem(matches []*Match){
	records := [][]string{{"Filename", "Description", "Filepath", "CommitID"}}
	for _, match := range matches {
		records = append(records, []string{match.Filename, match.Description, match.Filepath})
	}
	outputCSV(*outputCSVFlag, records)
}

func outputCSVGitRepo(matches []*Match){
	records := [][]string{{"Filename", "Description", "Filepath", "CommitID", "Value"}}
	for _, match := range matches {
		records = append(records, []string{match.Filename, match.Description, match.Filepath,
										   strings.Join(match.CommitIds, "|"), match.Value})
	}
	outputCSV(*outputCSVFlag, records)
}


func main() {
	initalize()
	flag.Parse()
	if len(*dirToScanFlag) != 0 {
		files, err := GetAllFilesInDirectory(*dirToScanFlag)
		if err != nil {
			fmt.Println(err)
			os.Exit(-1)
		}
		results := filesystemSecretFilenameLiteralSearch(files)
		results = append(results, filesystemSecretFilenameRegexSearch(files)...)
		if *outputCSVFlag {
			outputCSVFilesystem(results)
		}
	} else if len(*repoToScanFlag) != 0 {
		results, err := gitRepoSearch(*repoToScanFlag)
		if err != nil{
			fmt.Println(err)
		}
		if len(*outputCSVFlag) != 0 {
			outputCSVGitRepo(results)
		}
	} else if len(*organizationFlag) != 0{
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
		if *outputCSVFlag {
			outputCSVGitRepo(results)
		}
	} else {
		flag.Usage()
		os.Exit(-1)
	}
}

