## 项目说明：
### 本项目是基于mediasoup v3 版本 webrtc m84版本 的sfu的js版本修改成的c++版本.
### 支持和js版本一样的完整demo功能，但是本项目是直接把worker进程代码集成 到了demo可以直接代码级别的调用;
### 支持VC2019，xcode，CLion的开发环境，可以直接断点调试workder代码;
### 本项目支持所有跨平台系统 WIN，Linux，Mac，嵌入式arm等系统，国产麒麟系统，支持嵌入式系统编译;
### 编译脚本支持跨平台编译，支持 WIN，Linux，Mac，嵌入式arm等系统，国产麒麟系统

## 一、用vc2019编译，更新到最新版本
### win下使用vc2019自带安装的编译器 LLVM-clang-cl编译
### 直接执行如下命令生成vc2019工程文件

```
.\genvs2019.bat
```

### 注意：所有工程右键选择属性，选择编译器为vc2019自带安装的编译器 LLVM-clang-cl 编译器进行编译

生成工程文件即可打开工程编译  
然后拷贝  lib\win\x64\libssl-1_1-x64.dll  
        lib\win\x64\libcrypto-1_1-x64.dll  
到执行文件所在Debug目录下即可


## 二、MAC下xcode编译

### 1、建立依赖库

```
./builddeps.sh 
```
生成的执行文件生成在 
release/bin 目录下

### 2、生成用xcode开发环境编译调试
```
./genxcode.sh
```

## 三、Linux下编译
```
./builddeps.sh 
./build.sh
```


## 四、设置配置文件

### 配置文件拷贝的编译的执行文件所在目录
### 配置文件在源码根目录下 demo/config.json 拷贝到 xcode/debug 目录下和执行文件同一个目录
### vc2019 需要拷贝到对应的Debug目录下，和执行文件同一个目录
### 详细配置可以参考mediasoup官方demo的配置
### 配置文件里面的ip地址需要配置对应服务器的内网地址和外网地址
"ip"          : "192.168.1.192",//内网网卡地址  
"announcedIp" : "192.168.1.192"//公网地址


## 五、测试客户端输入地址
目前客户端代码还没整理好暂未开源后续会陆续开源
测试客户端下载地址
win下载地址：   
<http://121.36.0.230:8012/latest/sft-demo-client-1.0.1.win.zip>

mac下载地址：   
<http://121.36.0.230:8012/latest/sft-demo-client-1.0.1.mac.zip>

会议号输入任意数字比如   
会议号：123456  
用户名任意： 测试1  
地址栏修改为:ip地址改成自己的本机地址  
ws://192.168.3.19:8001/  


效果图
![image](https://raw.githubusercontent.com/yanhua133/mediasoup-sfu-cpp/main/demo.jpg)


## 六、交流及技术支持
加微信：jamesyhua，备注：进群，我拉你入微信群
![image](https://github.com/yanhua133/mediasoup-sfu-cpp/blob/main/zuozhe.jpeg?raw=true)