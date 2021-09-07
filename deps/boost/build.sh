cur_dir=$(cd "$(dirname "$0")"; pwd)
parent_dir=$(dirname $(pwd))
main_dir=$(dirname ${parent_dir})
echo "cur_dir=${cur_dir}"

if [ -e boost_1_74_0 ]; then
   #/bin/rm -rf boost_1_74_0
   echo "/bin/rm -rf boost_1_74_0"
fi
if [ -e include ]; then
   #/bin/rm -rf include
   echo "/bin/rm -rf include"
fi
if [ -e lib ]; then
   #/bin/rm -rf lib
   echo "/bin/rm -rf lib"
fi
tar zxvf boost_1_74_0.tar.gz

#https://blog.csdn.net/10km/article/details/72900248

if [ $CROSS_COMPILE_CPU ] ; 
then
  if [ $CROSS_COMPILE_CPU = "ON" ] ; then 
   TOOLS="using gcc : : ${TOOLCHAIN_DIR}/bin/${TOOLCHAIN_PREFIX}-gcc ;"
   #TOOLS="using gcc : : /opt/gcc-linaro-6.5.0-2018.12-x86_64_aarch64-linux-gnu/bin/aarch64-linux-gnu-gcc"
   RAWTOOLS="using gcc ;"
   export BOOST_BUILD_PATH=${cur_dir}
   echo "${TOOLS}" >$BOOST_BUILD_PATH/user-config.jam
   cat $BOOST_BUILD_PATH/user-config.jam

   fi

else 
   echo   "not found CROSS_COMPILE_CPU"  
fi



cd boost_1_74_0
./bootstrap.sh --prefix=${main_dir}/release --with-libraries=system,thread,filesystem,regex,random,chrono,atomic,date_time

./b2 stage release link=static runtime-link=static
./b2 install
echo "./b2 install"