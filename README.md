# PwnDelorean

PwnDelorean is a credential seeker. It allows users to search local and remote git repositories history for
any developer secrets that were left over and in addition searches filesystems for the same secrets.

### Prerequisities
Requires Go runtime, currently using go version 1.8.3

## Getting Started

```bash
go build . 
./PwnDelorean -url https://github.com/k4ch0w/PwnDelorean.git
```

### Scan filesystem
```bash
./PwnDelorean -directory ~/Workspace
```

### Scan Organization 
```bash
./PwnDelorean -organization GitHub
```

### Scan repo with Creds
#### Plaintext
```bash
export GIT_USER=k4ch0w
export GIT_PASS=*********
./PwnDelorean -url https://github.com/k4ch0w/PwnDeloreanRepo.git -creds=plaintext
```
#### SSH
```bash
export GIT_USER=k4ch0w
export GIT_PRIV_KEY=~/.ssh/id_rsa
export GIT_PUB_KEY=~/.ssh/id_rsa.pub
#If needed 
export GIT_PASSPHRASE=******
./PwnDelorean -url https://github.com/k4ch0w/PwnDeloreanRepo.git -creds=ssh
```
### Additional Flags
* csv - The filename to output results in CSV format to I.E ~/results.csv
* fileNamesOnly - Only look for interesting filenames instead of parsing each file
* ignoreForkedRepos - When scanning an organization ignore repos they forked
* ignoreHighFalsePositives - Ignore searching for patterns that generally cause false positives, be careful with this flag it will miss things like AWS keys and Azure keys
    	

## Authors

* **Paul Ganea** - *Initial work*


## License
This project is licensed under the MIT License - see the [LICENSE.md](LICENSE.md) file for details

