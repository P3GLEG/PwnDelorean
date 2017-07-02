package main

import (
	"fmt"
	"path/filepath"
	"os"
	"io/ioutil"
	"encoding/json"
	"flag"
	"regexp"
	"time"
	"log"
	"strings"
)

type Pattern struct {
	Description string `json:description`
	SecretType  string  `json:type`
	Value       string       `json:value`
}

var files = []string{}
var secretFileNameLiterals = []Pattern{}
var secretFileNameRegexes = []Pattern{}
var fileContentLiterals = []Pattern{}
var fileContentRegexes = []Pattern{}
var regexes = []*regexp.Regexp{}

func timeTrack(start time.Time, name string) {
	elapsed := time.Since(start)
	log.Printf("%s took %dms", name, elapsed.Nanoseconds()/1000)
}

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

func appendToFileArray(path string, info os.FileInfo, err error) error {
	files = append(files, path)
	return nil
}

func secretFilenameLiteralSearch() {
	for _, pattern := range secretFileNameLiterals {
		for _, filename := range files {
			if strings.Contains(filename, pattern.Value) {
				fmt.Println(fmt.Sprintf("Found match %s in %s", pattern.Description, filename))
			}
		}
	}
}

func secretFilenameRegexSearch() {
	for _, filename := range files {
		for _, reg := range regexes {
			if reg.MatchString(filename) {
				fmt.Println(fmt.Sprintf("Found match %s", filename))
				break
			}
		}
	}
}
func initalize(dirToScan string) {
	err := filepath.Walk("./patterns", initializePatterns)
	if err != nil {
		fmt.Println(err)
	}
	err = filepath.Walk(dirToScan, appendToFileArray)
	if err != nil {
		fmt.Println(err)
	}
	for _, pattern := range secretFileNameRegexes {
		temp := regexp.MustCompile(pattern.Value)
		regexes = append(regexes, temp)
	}
}

func main() {
	var dirToScan string
	flag.StringVar(&dirToScan, "directory", "", "Directory to scan")
	flag.Parse()
	if len(dirToScan) <= 0 {
		fmt.Println("Please provide a directory to scan")
		os.Exit(-1)
	}
	initalize(dirToScan)
	secretFilenameLiteralSearch()
	secretFilenameRegexSearch()
}


