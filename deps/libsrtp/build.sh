cur_dir=$(cd "$(dirname "$0")"; pwd)
parent_dir=$(dirname $(pwd))
main_dir=$(dirname ${parent_dir})

echo "cur_dir = ${cur_dir}"
echo "parent_dir = ${parent_dir}"
echo "main_dir = ${main_dir}"

/bin/rm -rf build
/bin/rm -rf CMakeCache.txt
/bin/rm -rf cmake_install.cmake

#if [ $CROSS_COMPILE_CPU ];then
#  ./configure "CFLAGS=${CMAKE_C_FLAGS}" --prefix=${cur_dir}/release  --enable-openssl --with-openssl-dir=${main_dir}/release
#else
mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX=${main_dir}/release \
      -DCMAKE_BUILD_TYPE=Release                  \
      -DENABLE_OPENSSL=ON                         \
      -DBUILD_SHARED_LIBS=OFF                     \
      -DOPENSSL_ROOT_DIR=${main_dir}/release/lib      \
      -DOPENSSL_INCLUDE_DIR=${main_dir}/release/include      \
      ..
#fi

make
make install