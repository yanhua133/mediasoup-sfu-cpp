Project Description: this project is a C + + version modified from the JS version of SFU of mediasoup V3 version and webrtc M84 version. It supports the same complete demo function as the JS version. However, this project directly integrates the worker process code into the demo, which can be called directly at the code level, and supports the development environment of vc2019, Xcode and clion, This project supports all cross platform systems win, Linux, MAC, embedded arm and other systems, domestic Kirin system, embedded system compilation, compilation instructions, compilation scripts, cross platform compilation, win, Linux, MAC, embedded arm and other systems, and domestic Kirin system

Compilation instructions: the compiled script supports cross platform compilation, supports win, Linux, MAC, embedded arm and other systems, and domestic Kirin system 1. Download the dependent library boost https://jaist.dl.sourceforge.net/project/boost/boost/1.74.0/boost_ 1_ 74_ 0.tar.gz is placed in the Lib directory. Remember that there is no need to decompress the compression, and the compiled script will be decompressed automatically. 1) the default MAC is compiled by clang, and Ubuntu is compiled by GCC. The compiler is installed according to the system default. The GCC version supports at least C + + 11, and the version is greater than > = gcc-6.1 3). Others are compiled through the source code of the DEPs directory, and there are no other dependent libraries

2. If the arm platform variable setting is cross platform compilation, the following scripts can be executed only once on the corresponding platform (the compiled include and lib are automatically installed in the release directory and called by the main program). Cross platform compilation requires the operation of x64 bit platform first, which can be ignored

source ./setarmenv.sh

3. Dependent library compilation (only compiled once)

./builddeps.sh

4. Compile the main program. / build.sh

The generated executable file is generated in the release / bin directory

5. You can also generate Xcode, compile and debug with Xcode development environment. / genxcode.sh

6. You can also compile and debug with clion. Note that sometimes there will be residual information in the updated clion, resulting in failure to compile successfully. At this time, you need to delete cmake_ Debug directory, then execute invalidate through file, restart clion, then re import the project, compile it, and directly open the project source directory

7. If you compile and execute with vc2019. \ genvs2019.bat generates a project file, you can open the project compilation

8. The directory where the compiled executable file of the configuration file copy is located. The configuration file is copied to the Xcode / debug directory under the root directory of the source code, or the cmakebuild directory of clion, which is the same directory as the executable file. For detailed configuration, please refer to the configuration file of the official demo of mediasoup. The IP address in the configuration file needs to be configured with the intranet address and Internet address "IP" of the corresponding server : "192.168.1.192", / / Intranet card address "announcedip": "192.168.1.192" / / public network address

9. Problems and solutions at present, only the following calls in the whole project are C + 14 standards, and others are C + + 11 standards. If there is a requirement for embedded development standards, it can be changed to C + + 11 const rtpcapabilities supportedrtpcapabilities=

Generating Xcode projects using cmake

cmake .. -G "Xcode"

The results report the following error

-- The C compiler identification is unknown

-- The CXX compiler identification is unknown

CMake Error at CMakeLists.txt:2 (project):

No CMAKE_ C_ COMPILER could be found.

CMake Error at CMakeLists.txt:2 (project):

No CMAKE_ CXX_ COMPILER could be found.

Later, use the following command to solve the problem

Sudo Xcode select -- switch / applications / xcode.app/ sudo Xcode select -- switch / applications / xcode.app/contents/developer remember to empty the files under build when trying again

Project Description: this project is a C + + version modified from the JS version of SFU of mediasoup V3 version and webrtc M84 version. It supports the same complete demo function as the JS version. However, this project directly integrates the worker process code into the demo, which can be called directly at the code level, and supports the development environment of vc2019, Xcode and clion, This project supports all cross platform systems win, Linux, MAC, embedded arm and other systems, domestic Kirin system, embedded system compilation, compilation instructions, compilation scripts, cross platform compilation, win, Linux, MAC, embedded arm and other systems, and domestic Kirin system

Compilation instructions: the compiled script supports cross platform compilation, supports win, Linux, MAC, embedded arm and other systems, and domestic Kirin system 1. Download the dependent library boost https://jaist.dl.sourceforge.net/project/boost/boost/1.74.0/boost_ 1_ 74_ 0.tar.gz is placed in the Lib directory. Remember that there is no need to decompress the compression, and the compiled script will be decompressed automatically. 1) the default MAC is compiled by clang, and Ubuntu is compiled by GCC. The compiler is installed according to the system default. The GCC version supports at least C + + 11, and the version is greater than > = gcc-6.1 3). Others are compiled through the source code of the DEPs directory, and there are no other dependent libraries

2. If the arm platform variable setting is cross platform compilation, the following scripts can be executed only once on the corresponding platform (the compiled include and lib are automatically installed in the release directory and called by the main program). Cross platform compilation requires the operation of x64 bit platform first, which can be ignored

source ./setarmenv.sh

3. Dependent library compilation (only compiled once)

./builddeps.sh

4. Compile the main program. / build.sh

The generated executable file is generated in the release / bin directory

5. You can also generate Xcode, compile and debug with Xcode development environment. / genxcode.sh

6. You can also compile and debug with clion. Note that sometimes there will be residual information in the updated clion, resulting in failure to compile successfully. At this time, you need to delete cmake_ Debug directory, then execute invalidate through file, restart clion, then re import the project, compile it, and directly open the project source directory

7. If you compile and execute with vc2019. \ genvs2019.bat generates a project file, you can open the project compilation

8. The directory where the compiled executable file of the configuration file copy is located. The configuration file is copied to the Xcode / debug directory under the root directory of the source code, or the cmakebuild directory of clion, which is the same directory as the executable file. For detailed configuration, please refer to the configuration file of the official demo of mediasoup. The IP address in the configuration file needs to be configured with the intranet address and Internet address "IP" of the corresponding server : "192.168.1.192", / / Intranet card address "announcedip": "192.168.1.192" / / public network address

9. Problems and solutions at present, only the following calls in the whole project are C + 14 standards, and others are C + + 11 standards. If there is a requirement for embedded development standards, it can be changed to C + + 11 const rtpcapabilities supportedrtpcapabilities=

Generating Xcode projects using cmake

cmake .. -G "Xcode"

The results report the following error

-- The C compiler identification is unknown

-- The CXX compiler identification is unknown

CMake Error at CMakeLists.txt:2 (project):

No CMAKE_ C_ COMPILER could be found.

CMake Error at CMakeLists.txt:2 (project):

No CMAKE_ CXX_ COMPILER could be found.

Later, use the following command to solve the problem

Sudo Xcode select -- switch / applications / xcode.app/ sudo Xcode select -- switch / applications / xcode.app/contents/developer remember to empty the files under build when trying again

