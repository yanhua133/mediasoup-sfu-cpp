cur_dir=$(cd "$(dirname "$0")"; pwd)
parent_dir=$(dirname $(pwd))

if [ -e deps/boost/boost_1_74_0.tar.gz ]; then
   echo "exist deps/boost/boost_1_74_0.tar.gz"
else
   /bin/cp -rf lib/boost_1_74_0.tar.gz  deps/boost/boost_1_74_0.tar.gz
fi

cd ${parent_dir}/deps/boost/
./build.sh
#sudo apt-get install libboost-all-dev