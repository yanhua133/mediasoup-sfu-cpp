cur_dir=$(cd "$(dirname "$0")"; pwd)
parent_dir=$(dirname $(pwd))
main_dir=$(dirname ${parent_dir})

echo "cur_dir = ${cur_dir}"
echo "parent_dir = ${parent_dir}"
echo "main_dir = ${main_dir}"

/bin/rm -rf build
/bin/rm -rf CMakeCache.txt
/bin/rm -rf cmake_install.cmake

mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX=${main_dir}/release  \
      -DMAIN_DIR=${main_dir}                      \
      -DPROJECT_LIB_DIR=${main_dir}/release/lib      \
      -DPROJECT_INCLUDE_DIR=${main_dir}/release/include      \
      ..
make
make install