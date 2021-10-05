cur_dir=$(cd "$(dirname "$0")"; pwd)
parent_dir=$(dirname $(pwd))
cd script
#./buildzlib.sh
./buildopenssl.sh
#./buildboringssl.sh
./buildboost.sh
./buildoatpp.sh
#./buildspdlog.sh
./builduv.sh
./buildjson.sh
./buildnetstring.sh
./buildcatch.sh
./buildsrtp.sh
./buildusrsctp.sh
./buildboost.sh
./buildabsl.sh
./buildsdp.sh
./buildwebrtc.sh


