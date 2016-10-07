# PwnDelorean 

PwnDelorean is a credential seeker. It allows users to search local and remote git repositories history for 
any developer secrets that were left over and in addition searches filesystems for the same secrets.

## Getting Started
```

./pwndelorean -r https://github.com/pegleg2060/pwndelorean.git -o /tmp/whatever
```

### Prerequisities
This currently is known to run on Kali 4.3, future support for other platforms is coming

### Installing

The program currently uses Cmake to build and requires some dependencies 

```
git clone https://github.com/pegleg2060/PwnDelorean.git
cd PwnDelorean
git submodule update --init --recursive
mkdir build
cd build
cmake ..
make
```

## Running the tests
I am adding tests as a I go along, follow these instructions to build them
```
cd test
mkdir build
cd build
cmake ..
make
./runTests
```
### Test Cases
The tests are built to ensure that the Regexes created are able to find the content it
is intended for as an example you might want a .pfx file but it could be mutated
```
somefilename.pfx
.hidden.pfx
anotherfilename.pfx.backup
anotherfilename.pfx.temp
```
Whenever a new pattern is added please ensure you write a test suite to find these cases

## Authors

* **Paul Ganea** - *Initial work* - [Casaba ](https://casaba.com)


## License
This project is licensed under the MIT License - see the [LICENSE.md](LICENSE.md) file for details

## Acknowledgments
* Dennis Ritchie 
* Kobe Bryant
* Wiz Khalifa


