cur_dir=$(cd "$(dirname "$0")"; pwd)
parent_dir=$(dirname $(pwd))

if [ -e ${parent_dir}/deps/boost/boost_1_74_0.tar.gz ]; then
   echo "exist deps/boost/boost_1_74_0.tar.gz"
else
   /bin/cp -rf ${parent_dir}/lib/boost_1_74_0.tar.gz  ${parent_dir}/deps/boost/boost_1_74_0.tar.gz
fi

cd ${parent_dir}/deps/boost/
./build.sh
#sudo apt-get install libboost-all-dev