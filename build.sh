cur_dir=$(cd "$(dirname "$0")"; pwd)
parent_dir=$(dirname $(pwd))

if [ -e /etc ]; then
   /bin/rm -rf build
   /bin/rm -rf CMakeCache.txt
   /bin/rm -rf cmake_install.cmake
else
   echo "is window"
   rm  build -recurse
   rm  CMakeCache.txt
   rm  cmake_install.cmake
fi

if [ -e release/ ]; then
  #/bin/rm -rf release/bin/*
  echo "not rm"
fi

mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX=${cur_dir}/release ..
make
make install