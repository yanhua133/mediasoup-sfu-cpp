项目说明：
本项目是基于mediasoup的sfu的js版本修改成的c++版本，支持和js版本一样的完整demo功能，但是本项目是直接把worker进程代码集成
到了demo可以直接代码级别的调用，并可以直接断点调试workder代码
本项目支持所有跨平台系统 WIN，Linux，Mac，嵌入式arm等系统，国产麒麟系统，支持嵌入式系统编译

编译说明，编译脚本支持跨平台编译，支持 WIN，Linux，Mac，嵌入式arm等系统，国产麒麟系统
1、下载依赖库boost 
https://jaist.dl.sourceforge.net/project/boost/boost/1.74.0/boost_1_74_0.tar.gz
放入lib目录下，记住不需要解解压缩，编译脚本会自动解压
1)、默认mac下用的是clang编译，ubuntu用的是gcc编译，按照系统默认安装编译器就行，gcc版本至少支持c++11版本大于 >= gcc-6.1
3)、其他的都是通过deps目录的源码进行编译的，再没有其他依赖库

2、arm平台变量设置
如果是跨平台编译，下述脚本只在对应平台执行一次即可（编译后的include和lib都自动安装到了release目录下用主程序调用）
跨平台编译需要先执行
x64位平台需要操作，可以忽略

source ./setarmenv.sh

3、依赖库编译（只编译一次）

./builddeps.sh 

4、编译主程序
./build.sh

生成的执行文件生成在 
release/bin 目录下

5、也可以生成xcode，用xcode开发环境编译调试
./genxcode.sh

6、也可以用clion编译调试
注意有时候更新后clion会有残留信息导致无法编译成功，此时需要删除cmake_debug目录，然后通过file执行invalidate，重启clion，然后重新导入工程，编译就行了
直接打开项目源码目录即可

7、如果用vc2019编译执行
.\genvs2019.bat
生成工程文件即可打开工程编译

8、配置文件拷贝的编译的执行文件所在目录
配置文件在源码根目录下 demo/config.json 拷贝到 xcode/debug 目录下 或是clion的cmakebuild目录下和执行文件同一个目录
详细配置可以参考mediasoup官方demo的配置
配置文件里面的ip地址需要配置对应服务器的内网地址和外网地址
"ip"          : "192.168.1.192",//内网网卡地址
"announcedIp" : "192.168.1.192"//公网地址


9、问题及解决
目前整个项目只有下面的调用是c+14标准，其他都是c++11标准，后续如果有嵌入开发标准需求可以改成c++11
const RtpCapabilities supportedRtpCapabilities =

使用cmake生成xcode的项目

cmake .. -G "Xcode"

结果报如下错误

-- The C compiler identification is unknown

-- The CXX compiler identification is unknown

CMake Error at CMakeLists.txt:2 (project):

 No CMAKE_C_COMPILER could be found.

CMake Error at CMakeLists.txt:2 (project):

 No CMAKE_CXX_COMPILER could be found.

后来使用如下命令解决

sudo xcode-select --switch /Applications/Xcode.app/
sudo xcode-select --switch /Applications/Xcode.app/Contents/Developer
重试时记得清空build下的文件
#


10、交流及技术支持
加微信：jamesyhua，备注：进群，我拉你入微信群
![image](https://github.com/yanhua133/mediasoup-sfu-cpp/blob/main/zuozhe.jpeg?raw=true)