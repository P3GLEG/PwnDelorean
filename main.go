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
}

var secretFileNameLiterals = []Pattern{}
var secretFileNameRegexes = []Pattern{}
var fileContentLiterals = []Pattern{}
var fileContentRegexes = []Pattern{}
var regexes = []*regexp.Regexp{}

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
				secretFileNameRegexes = append(secretFileNameRegexes, pattern)
			case "fileContentLiteral":
				fileContentLiterals = append(fileContentLiterals, pattern)
			case "fileContentRegex":
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

func secretFilenameLiteralSearch(files []os.FileInfo) {
	for _, pattern := range secretFileNameLiterals {
		for _, filename := range files {
			if strings.Contains(filename.Name(), pattern.Value) {
				fmt.Println(fmt.Sprintf("Found match %s in %s", pattern.Description, filename.Name()))
			}
		}
	}
}

func secretFilenameRegexSearch(files []os.FileInfo) {
	for _, filename := range files {
		for _, reg := range regexes {
			if reg.MatchString(filename.Name()) {
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
	for _, pattern := range secretFileNameRegexes {
		temp := regexp.MustCompile(pattern.Value)
		regexes = append(regexes, temp)
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
		secretFilenameLiteralSearch(files)
		secretFilenameRegexSearch(files)
	} else if len(*repoToScanFlag) != 0 {
		yay, err := GetRepoFilenames(*repoToScanFlag)
		if err != nil {
			fmt.Println(err)
		}
		fmt.Println(yay)
	} else {
		flag.Usage()
		os.Exit(-1)
	}

	//secretFilenameLiteralSearch()
	//secretFilenameRegexSearch()
	/*

	*/
}
