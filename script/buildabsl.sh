cur_dir=$(cd "$(dirname "$0")"; pwd)
parent_dir=$(dirname $(pwd))

cd ${parent_dir}/deps/libwebrtc/deps/abseil-cpp
./build.sh