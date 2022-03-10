
## [中文说明请点击](https://github.com/yanhua133/mediasoup-sfu-cpp/blob/main/README.cn.md)
## [gitee地址](https://gitee.com/yanhua133/mediasoup-sfu-cpp)
##  Project Description:

###  This project is a C + + version based on the JS version of SFU in mediasoup V3, webrtc M77

###  It supports the same complete demo function as the JS version, but this project directly integrates the worker process code into the demo, which can be called directly at the code level;

###  Support the development environment of vc2019, Xcode and clion, and can directly debug the worker code at breakpoints;

###  The project supports all cross platform systems win, Linux, MAC, embedded arm and other systems, domestic Kirin system and embedded system compilation;

###  The compiled script supports cross platform compilation, win, Linux, MAC, embedded arm and other systems, and domestic Kirin system


##  1、Compile with vc2019 and update to the latest version

###  Under win, compile with the compiler llvm clang CL installed with vc2019

###  Directly execute the following command to generate vc2019 project file


```
.\genvs2019.bat
```
###  Note: right click all projects, select properties, and select the compiler llvm clang CL compiler installed with vc2019 to compile


Generate a project file to open the project compilation

Then copy   

lib\win\x64\libssl-1_1-x64.dll  
lib\win\x64\libcrypto-1_1-x64.dll

Go to the debug directory where the execution file is located



##  2、 Xcode compilation under mac


###  1). Establish dependency Library

```
./builddeps.sh
```
Generated executable

Release /bin directory

###  2). Compile and debug with Xcode development environment

```
./genxcode.sh
```

##  3、 Compiling under Linux gcc >= 8.4.0

```
./builddeps.sh
./build.sh
```


##  4、 Set config

###  Directory where the compiled executable file of the configuration file copy is located

###  The configuration file is in the root directory of the source code, demo / config Copy JSON to the Xcode / debug directory and the same directory as the execution file

###  Vc2019 needs to be copied to the corresponding debug directory, which is the same directory as the execution file

###  For detailed configuration, please refer to the configuration of the official mediasoup demo

###  The IP address in the configuration file needs to be configured with the intranet address and extranet address of the corresponding server

"IP": "192.168.1.192"  
"Announcedip": "192.168.1.192" / / public network address



##  5、 Test client input address

At present, the client code has not been sorted out yet, and it will be open source in the future

Test client download address

Win download address:

http://121.36.0.230:8012/latest/sft-demo-client-1.0.1.win.zip

Mac download address:

http://121.36.0.230:8012/latest/sft-demo-client-1.0.1.mac.zip

Enter any number for the conference number, such as

Conference No.: 123456

User name: Test 1

The address bar is modified as follows: change the IP address to your own local address
ws://192.168. 3.19:8001/

design sketch

![image]( https://raw.githubusercontent.com/yanhua133/mediasoup-sfu-cpp/main/demo.jpg )


##  6、Communication and technical support

Add wechat: James yhua, note: join the group, I'll pull you into the wechat group

![image](https://github.com/yanhua133/mediasoup-sfu-cpp/blob/main/zuozhe.jpeg?raw=true)

## 7、Discord discussion group

<https://discord.gg/f55DnczkEx>