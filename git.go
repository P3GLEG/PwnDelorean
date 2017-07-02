package main

import (
	"gopkg.in/libgit2/git2go.v25"
	"log"
)

func repo(){
	_, err := git.OpenRepository("https://github.com/k4ch0w/PwnDelorean")
	if err != nil {
		log.Fatal(err)
	}
}

