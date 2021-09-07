cur_dir=$(cd "$(dirname "$0")"; pwd)
parent_dir=$(dirname $(pwd))
main_dir=$(dirname ${parent_dir})

echo "cur_dir = ${cur_dir}"
echo "parent_dir = ${parent_dir}"
echo "main_dir = ${main_dir}"

/bin/rm -rf build
/bin/rm -rf CMakeCache.txt
/bin/rm -rf cmake_install.cmake

#mkdir build
#cd build
#cmake -DCMAKE_INSTALL_PREFIX=${main_dir}/release ..
chmod 777 config


if [ ${CROSS_COMPILE} ]; then
echo "${CROSS_COMPILE}"
./config no-asm no-async no-shared no-dso --prefix=${main_dir}/release --cross-compile-prefix=${CROSS_COMPILE}
sed -i 's/\-m64//g' ./Makefile
else

echo "${CROSS_COMPILE}"
./config --prefix=${main_dir}/release
fi

make
make install