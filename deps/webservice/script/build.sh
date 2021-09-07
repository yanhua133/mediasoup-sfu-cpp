#!/bin/bash

# exit script on any error
set -e
set -o xtrace

PROJECT_DIR=$(pwd)
INSTALL_PATH=$PROJECT_DIR/../tmp/usr
mkdir -p $INSTALL_PATH

# Configure Project
mkdir -p $PROJECT_DIR/build
cd $PROJECT_DIR/build
cmake -DCMAKE_INSTALL_PREFIX=$INSTALL_PATH -DWEBSERVICE_BUILD_TESTS=ON -DWEBSERVICE_BUILD_EXAMPLES=ON ..

# Build tests and examples
make

# Run tests
# TODO

# Run examples
# TODO

# Install
make install

# Check install
git diff --no-index $INSTALL_PATH/include/webservice $PROJECT_DIR/include/webservice

CMAKE_FIND_FILE=$INSTALL_PATH/lib/cmake/webservice/webservice-config.cmake
if [ -a $CMAKE_FIND_FILE ]
then
    echo "$CMAKE_FIND_FILE - Found"
else
    echo "$CMAKE_FIND_FILE - Not found"
    false
fi

# Check install usage by example project
mkdir -p $PROJECT_DIR/build-package-test
cd $PROJECT_DIR/build-package-test
cmake -DCMAKE_PREFIX_PATH=$INSTALL_PATH ../test-package
make
./test_webservice_package
