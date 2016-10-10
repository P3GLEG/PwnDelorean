CURDIR=`pwd`
git submodule update --init --recursive
mkdir -p $CURDIR/deps/libgit2/build
cd $CURDIR/deps/libgit2/build
cmake -DTHREADSAFE=ON -DBUILD_CLAR=OFF -DBUILD_SHARED_LIBS=OFF -DCMAKE_C_FLAGS=-fPIC -DCMAKE_BUILD_TYPE="RelWithDebInfo" -DCMAKE_INSTALL_PREFIX=../install -DCURL=OFF .. && cmake --build .
mkdir -p $CURDIR/deps/re2/build
cd $CURDIR/deps/re2/build
make test
make install
make testinstall
cd $CURDIR
mkdir build
cd build
cmake ..
make
