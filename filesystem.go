package main

import (
	"fmt"
	"path/filepath"
	"os"
	"strings"
	"io/ioutil"
	"bytes"
)


type FileStruct struct {
	Filename string
	Path string
}

var ignoreExts = []string{".dll", ".nupkg"}

func appendFilesystemMatch(pattern Pattern, filestruct FileStruct, lineMatched string, table map[string]*Match) {
	name := filestruct.Filename
	_, exists := table[name]
	if !exists {
		table[name] = &Match{filestruct.Filename, filestruct.Path,
			nil,pattern.Description, pattern.Value, lineMatched}
		fmt.Println(fmt.Sprintf("Found match %s: %s %s", pattern.Description, filestruct.Path, pattern.Value))
	}
}
func getAllFilesInDirectory(dir string) ([]FileStruct, error) {
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

func searchThroughFile(path string) (bool, Pattern, string, error){
    fi, err := os.Stat(path)
    if err != nil {
        return false, Pattern{}, "", err
    }
    ext := filepath.Ext(path)
    for _, ignore := range ignoreExts{
        if ext == ignore{
            return false, Pattern{}, "", err
        }
    }
    if fi.IsDir() || strings.Contains(path, ".git"){ //Tons of false positives in .git folder
        return false, Pattern{}, "", err
    }
    f, err := ioutil.ReadFile(path)
    if err != nil{
        return false, Pattern{}, "", err
    }

    lines := bytes.Split(f, []byte("\n"))
    for _, pattern := range fileContentRegexes{
        for _, line := range lines{
            if pattern.Regex.Match(line){
                return true, pattern, string(line), nil
            }
        }
        }
    return false, Pattern{}, "", nil
}
func searchFileContents(files []FileStruct, table map[string]*Match){
        for _, f := range files{
            match, p, line, err :=searchThroughFile(f.Path)
            if err == nil && match{
                appendFilesystemMatch(p,f, line,table)
            }
        }
}

func outputCSVFilesystem(matches []*Match){
	records := [][]string{{"Filename", "Description", "Filepath", "LineMatched"}}
	for _, match := range matches {
		records = append(records, []string{match.Filename, match.Description,
            match.Filepath, truncateString(match.LineMatched, 160)})
	}
	outputCSV(*outputCSVFlag, records)
}

func filesystemSecretFilenameRegexSearch(files []FileStruct, table map[string]*Match){
	for _, filestruct := range files {
		for _, pattern := range secretFileNameRegexes {
			if pattern.Regex.MatchString(filestruct.Filename) {
				appendFilesystemMatch(pattern, filestruct, "", table)
				break
			}
		}
	}
}


func filesystemSecretFilenameLiteralSearch(files []FileStruct, table map[string]*Match) {
	for _, pattern := range secretFileNameLiterals {
		for _, filename := range files {
			if strings.Contains(filename.Filename, pattern.Value) {
				appendFilesystemMatch(pattern, filename, "", table)
			}
		}
	}

}

func startFileSystemScan(){
	files, err := getAllFilesInDirectory(*dirToScanFlag)
		if err != nil {
			fmt.Println(err)
			os.Exit(-1)
		}
        var uniqFileSet = make(map[string]*Match)
		filesystemSecretFilenameLiteralSearch(files, uniqFileSet)
		filesystemSecretFilenameRegexSearch(files, uniqFileSet)
        if !*fileNamesOnlyFlag{
            searchFileContents(files, uniqFileSet)
        }
        if len(*outputCSVFlag) != 0 {
            var results []*Match
            for _, values := range uniqFileSet{
                results = append(results, values)
            }
			outputCSVFilesystem(results)
		}
}