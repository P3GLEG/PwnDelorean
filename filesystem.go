package main

import (
	"fmt"
	"path/filepath"
	"os"
	"strings"
	"io/ioutil"
	"bytes"
	"github.com/fatih/color"
	"strconv"
	"github.com/pkg/errors"
)

type FileStruct struct {
	Filename string
	Path     string
}

const MAX_FILE_SIZE_TO_SCAN = 200000
var ignoreExts = []string{".dll", ".nupkg", ".mp4", ".mkv", ".avi", ".flv", ".wmv", ".mov", ".pdf" }

func appendFilesystemMatch(pattern Pattern, filestruct FileStruct, lineMatched string,
	lineNum int, table map[string]*Match) {
	name := filestruct.Filename
	_, exists := table[name]
	if !exists {
		table[name] = &Match{filestruct.Filename, filestruct.Path,
							 nil, pattern.Description,
			pattern.Value, lineMatched, lineNum}
		fmt.Println(color.BlueString("Found match %s %s: ", pattern.Description, pattern.Value))
		if lineNum == NO_LINE_NUMBER_APPLICABLE{
			color.Green("%s", filestruct.Path)
		} else{
			color.Green("%s:%d ", filestruct.Path, lineNum)
		}
	}
}
func getAllFilesInDirectory(dir string) ([]FileStruct, error) {
	fileList := []FileStruct{}
	err := filepath.Walk(dir, func(path string, f os.FileInfo, err error) error {
		fileList = append(fileList, FileStruct{f.Name(), path})
		return nil
	})
	if err != nil {
		return nil, err
	}
	return fileList, nil
}

func searchThroughFile(path string) (bool, Pattern, string, int, error) {
	fi, err := os.Stat(path)
	if err != nil {
		return false, Pattern{}, "", -1, err
	}
	ext := filepath.Ext(path)
	for _, ignore := range ignoreExts {
		if ext == ignore {
			return false, Pattern{}, IS_FILENAME_MATCH,
				NO_LINE_NUMBER_APPLICABLE, errors.New("Ignoring file extension " + path)
		}
	}
	if fi.IsDir() || checkIfInsideIgnoredDirectory(path) {
		return false, Pattern{}, IS_FILENAME_MATCH,
			NO_LINE_NUMBER_APPLICABLE, err
	}
	if fi.Size() / 1024 >= MAX_FILE_SIZE_TO_SCAN{
		return false, Pattern{}, IS_FILENAME_MATCH,
			NO_LINE_NUMBER_APPLICABLE, errors.New(path + " is over max file size to scan")
	}
	f, err := ioutil.ReadFile(path)
	if err != nil {
		return false, Pattern{}, IS_FILENAME_MATCH,
			NO_LINE_NUMBER_APPLICABLE, err
	}
	lines := bytes.Split(f, []byte("\n"))
	for _, pattern := range fileContentRegexes {
		lineNum := 0
		for _, line := range lines {
			lineNum++
			if pattern.Regex.Match(line) {
				return true, pattern, string(line), lineNum, nil
			}
		}
	}
	return false, Pattern{}, IS_FILENAME_MATCH, NO_LINE_NUMBER_APPLICABLE, nil
}
func searchFileContents(files []FileStruct, table map[string]*Match) {
	for _, f := range files {
		match, p, line, lineNum, err := searchThroughFile(f.Path)
		if err == nil && match {
			appendFilesystemMatch(p, f, line, lineNum, table)
		}
	}
}

func outputCSVFilesystem(matches []*Match) {
	records := [][]string{{"Filename", "Description", "Filepath", "LineMatched", "LineNumber"}}
	for _, match := range matches {
		records = append(records, []string{match.Filename, match.Description,
										   match.Filepath, truncateString(match.LineMatched, 160),
		strconv.Itoa(match.LineNumber)})
	}
	outputCSV(*outputCSVFlag, records)
}

func filesystemSecretFilenameRegexSearch(files []FileStruct, table map[string]*Match) {
	for _, filestruct := range files {
		for _, pattern := range secretFileNameRegexes {
			if pattern.Regex.MatchString(filestruct.Filename) {
				appendFilesystemMatch(pattern, filestruct, IS_FILENAME_MATCH, NO_LINE_NUMBER_APPLICABLE, table)
				break
			}
		}
	}
}

func filesystemSecretFilenameLiteralSearch(files []FileStruct, table map[string]*Match) {
	for _, pattern := range secretFileNameLiterals {
		for _, filename := range files {
			if strings.Contains(filename.Filename, pattern.Value) {
				appendFilesystemMatch(pattern, filename, IS_FILENAME_MATCH, NO_LINE_NUMBER_APPLICABLE, table)
			}
		}
	}
}

func startFileSystemScan() {
	files, err := getAllFilesInDirectory(*dirToScanFlag)
	if err != nil {
		color.Red(err.Error())
		os.Exit(-1)
	}
	var uniqFileSet = make(map[string]*Match)
	filesystemSecretFilenameLiteralSearch(files, uniqFileSet)
	filesystemSecretFilenameRegexSearch(files, uniqFileSet)
	if !*fileNamesOnlyFlag {
		searchFileContents(files, uniqFileSet)
	}
	if len(*outputCSVFlag) != 0 {
		var results []*Match
		for _, values := range uniqFileSet {
			results = append(results, values)
		}
		outputCSVFilesystem(results)
	}
}
