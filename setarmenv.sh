# 创建安装目录
#mkdir ~/App
#cur_dir=$(cd "$(dirname "$0")"; pwd)
# 解压安装包
#tar -xvf gcc-linaro-4.9.4-2017.01-x86_64_aarch64-linux-gnu.tar.xz -C ~/App

# 配置环境变量
#echo "" >> ~/.bashrc
# 当安装有多个交叉编译器时，则不能将CROSS_COMPILE配置到环境变量中，需要每次编译前设置CROSS_COMPILE环境变量
#echo "export CROSS_COMPILE=aarch64-linux-gnu-"  >> ~/.bashrc
#echo "export PATH=~/App/gcc-linaro-4.9.4-2017.01-x86_64_aarch64-linux-gnu/bin:\$PATH"  >> ~/.bashrc
export TOOLCHAIN_DIR=/opt/gcc-linaro-5.4.1-2017.05-rc1-x86_64_aarch64-linux-gnu
#export TOOLCHAIN_DIR=/opt/gcc-linaro-7.5.0-2019.12-x86_64_aarch64-linux-gnu
#/opt/gcc-linaro-6.5.0-2018.12-x86_64_aarch64-linux-gnu
export MAIN_PROJECT_DIR=${cur_dir}
export PATH=${TOOLCHAIN_DIR}/bin:$PATH
export CROSS_COMPILE=aarch64-linux-gnu-
export CC=${CROSS_COMPILE}gcc
export CXX=${CROSS_COMPILE}g++
export LINK=$CXX
export LD=${CROSS_COMPILE}ld
export AR=${CROSS_COMPILE}ar
export AS=${CROSS_COMPILE}as
export RANLIB=${CROSS_COMPILE}ranlib
export LD_LIBRARY_PATH=${TOOLCHAIN_DIR}/lib:$LD_LIBRARY_PATH
export TOOLCHAIN_PREFIX=aarch64-linux-gnu
export TOOLCHAIN_GNU_FLAGS="-fPIC -pthread"
export CROSS_COMPILE_CPU=ON
export CMAKE_SYSTEM_PROCESSOR="arm64"
export CPU_TYPE="arm"

#注意，直接 ./setarmenv.sh 运行不会生效
#需要通过 source ./setarmenv.sh 运行脚本环境变量才生效