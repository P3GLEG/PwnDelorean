package main

import (
	"fmt"
	"path/filepath"
	"os"
	"io/ioutil"
	"encoding/json"
	"flag"
	"regexp"
	"encoding/csv"
)

type Pattern struct {
	Description string `json:description`
	SecretType  string  `json:type`
	Value       string       `json:value`
	HighFalsePositive bool `json:highFalsePositive`
	Regex       *regexp.Regexp
}

type Match struct {
	Filename    string
	Filepath    string
	CommitIds   []string
	Description string
	Value string
    LineMatched string
}


var dirToScanFlag = flag.String("directory", "", "Directory to scan")
var repoToScanFlag = flag.String("url", "", "Git Repo URL to scan")
var outputCSVFlag = flag.String("csv", "", "Output in CSV Format")
var fileNamesOnlyFlag = flag.Bool("fileNamesOnly", false,
	"Disable searching through File for speed increase")
var organizationFlag= flag.String("organization", "", "Search all of an Organizations repos")
var ignoreForkRepos = flag.Bool("ignoreForkedRepos", false,
	"Ignore any Organization repos that are forked")
var ignoreHighFalsePositivePatterns = flag.Bool("ignoreHighFalsePositives", false,
	"Ignore patterns that cause a lot of false findings")

var secretFileNameLiterals = []Pattern{}
var secretFileNameRegexes = []Pattern{}
var fileContentLiterals = []Pattern{}
var fileContentRegexes = []Pattern{}

func initialize() {
	err := filepath.Walk("./patterns", initializePatterns)
	if err != nil {
		fmt.Println(err)
	}
}

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
			if *ignoreHighFalsePositivePatterns && pattern.HighFalsePositive{
				continue
				fmt.Println("Ignoring " + pattern.Value)
			}
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

func truncateString(str string, num int) string {
    temp := str
    if len(str) > num {
        if num > 3 {
            num -= 3
        }
        temp = str[0:num] + "..."
    }
    return temp
}

func main() {
	initialize()
	flag.Parse()
	if len(*dirToScanFlag) != 0 {
		startFileSystemScan()
	} else if len(*repoToScanFlag) != 0 {
		startGitRepoScan()
	} else if len(*organizationFlag) != 0{
		startGitOrganizationScan()
	} else {
		flag.Usage()
		os.Exit(-1)
	}
}

