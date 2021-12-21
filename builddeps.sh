cur_dir=$(cd "$(dirname "$0")"; pwd)
parent_dir=$(dirname $(pwd))
cd script
#./buildzlib.sh
#./buildopenssl.sh
#./buildboringssl.sh
#./buildboost.sh
#./buildlibressl.sh
./buildopenssl.sh
./buildoatpp.sh
./buildoatppwebsocket.sh
./buildoatppopenssl.sh
#./buildspdlog.sh
./builduv.sh
./buildjson.sh
./buildnetstring.sh
./buildcatch.sh
./buildsrtp.sh
./buildusrsctp.sh
#./buildboost.sh
./buildabsl.sh
./buildsdp.sh
./buildwebrtc.sh


