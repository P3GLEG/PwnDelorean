CURDIR=`pwd`
git submodule update --init --recursive
apt-get install cmake libgit2-dev libicu-dev libssl-dev pkg-config libssh2-1-dev libhttp-parser-dev libre2-dev -y
mkdir -p $CURDIR/deps/googletest/build
cd $CURDIR/deps/googletest/build
cmake .. && make
cd $CURDIR
mkdir -p $CURDIR/deps/libgit2/build
cd $CURDIR/deps/libgit2/build
cmake -DTHREADSAFE=ON -DBUILD_CLAR=OFF -DBUILD_SHARED_LIBS=OFF -DCMAKE_C_FLAGS=-fPIC -DCMAKE_BUILD_TYPE="RelWithDebInfo" -DCMAKE_INSTALL_PREFIX=../install -DCURL=OFF .. && cmake --build .
cd $CURDIR
mkdir build
cd build
cmake ..
make
mkdir -p /usr/share/PwnDelorean/
cp -r patterns /usr/share/PwnDelorean/
cp pwndelorean /usr/local/bin/
