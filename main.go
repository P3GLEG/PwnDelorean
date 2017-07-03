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
	CommitID    string
	Description string
}

var secretFileNameLiterals = []Pattern{}
var secretFileNameRegexes = []Pattern{}
var fileContentLiterals = []Pattern{}
var fileContentRegexes = []Pattern{}

func initializePatterns(path string, info os.FileInfo, err error) error {
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

func GetAllFilesInDirectory(dir string) ([]os.FileInfo, error) {
	files, err := ioutil.ReadDir(dir)
	return files, err
}

func gitSecretFilenameLiteralSearch(files []GitFile) []Match {
	var results []Match
	var table = make(map[string]bool)
	for _, pattern := range secretFileNameLiterals {
		for _, filename := range files {
			if strings.Contains(filename.Name, pattern.Value) {
				if table[filename.Name] {
					continue
				} else {
					table[filename.Name] = true
					path := filename.Filepath
					if len(path) == 0 {
						path = "/" + filename.Name
					} else {
						path += filename.Name
					}
					results = append(results, Match{filename.Name, path,
													filename.CommitId, pattern.Description})
					fmt.Println(fmt.Sprintf("Found match %s %s", pattern.Description, path))
				}
			}
		}
	}
	return results
}

func gitSecretFilenameRegexSearch(files []GitFile) []Match {
	var results []Match
	var table = make(map[string]bool)
	for _, filename := range files {
		for _, pattern := range secretFileNameRegexes {
			if pattern.Regex.MatchString(filename.Name) && !table[filename.Name] {
				path := filename.Filepath
				if len(path) == 0 {
					path = "/" + filename.Name
				} else {
					path += filename.Name
				}
				results = append(results, Match{filename.Name, path,
												filename.CommitId, pattern.Description})
				fmt.Println(fmt.Sprintf("Found match %s %s", pattern.Description, path))
				break
			}
		}
	}
	return results
}

func filesystemSecretFilenameLiteralSearch(files []os.FileInfo) {
	for _, pattern := range secretFileNameLiterals {
		for _, filename := range files {
			if strings.Contains(filename.Name(), pattern.Value) {
				fmt.Println(fmt.Sprintf("Found match %s in %s", pattern.Description, filename.Name()))
			}
		}
	}
}

func filesystemSecretFilenameRegexSearch(files []os.FileInfo) {
	for _, filename := range files {
		for _, pattern := range secretFileNameRegexes {
			if pattern.Regex.MatchString(filename.Name()) {
				fmt.Println(fmt.Sprintf("Found match %s", filename.Name()))
				break
			}
		}
	}
}

var dirToScanFlag = flag.String("directory", "", "Directory to scan")
var repoToScanFlag = flag.String("url", "", "Git Repo URL to scan")

func initalize() {
	err := filepath.Walk("./patterns", initializePatterns)
	if err != nil {
		fmt.Println(err)
	}
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
		filesystemSecretFilenameLiteralSearch(files)
		filesystemSecretFilenameRegexSearch(files)
	} else if len(*repoToScanFlag) != 0 {
		files, err := GetRepoFilenames(*repoToScanFlag)
		if err != nil {
			fmt.Println(err)
		}
		gitSecretFilenameLiteralSearch(files)
		gitSecretFilenameRegexSearch(files)
		//TODO: Output format for results
	} else {
		flag.Usage()
		os.Exit(-1)
	}
}
