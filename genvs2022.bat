@echo off
set CURRENT_DIR=%~dp0
set PROJECT_NAME=vc2022

echo %CURRENT_DIR%%PROJECT_NAME%
if not exist %CURRENT_DIR%%PROJECT_NAME% (
    mkdir %CURRENT_DIR%%PROJECT_NAME%
)
del /f/s/q CMakeCache.txt
cd %CURRENT_DIR%%PROJECT_NAME%

set OPENSSL_ROOT_DIR="C:\Program Files\OpenSSL-Win64"
rem Generate VS2019 project.
cmake ../ -G "Visual Studio 17 2022" 
pause